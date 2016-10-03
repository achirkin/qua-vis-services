#include "quavis/vk/instance.h"
#include "quavis/vk/device/physicaldevice.h"
#include "quavis/vk/device/logicaldevice.h"
#include "quavis/vk/memory/allocator.h"
#include "quavis/vk/memory/buffer.h"
#include "quavis/vk/memory/image.h"
#include "quavis/vk/memory/shader.h"
#include "quavis/vk/descriptors/descriptorpool.h"
#include "quavis/vk/descriptors/descriptorset.h"
#include "quavis/vk/pipeline/graphicspipeline.h"
#include "quavis/vk/pipeline/computepipeline.h"
#include "quavis/vk/geometry/geoemtry.h"
#include "quavis/shaders.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tinyobjloader.h"

#include <iostream>
#include <chrono>

std::vector<quavis::Vertex> vertices_ = {};

std::vector<uint32_t> indices_ = {};

uint32_t width = 2048;
uint32_t height = 1024;

struct UniformBufferObject {
  quavis::vec3 observation_point;
  float r_max;
  float alpha_max;
};

UniformBufferObject uniform_ = {
  {0.0,0.0,0.0},
  2.0,
  0.1
};

int main(int argc, char** argv) {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string err;
  std::string path = "/home/mfranzen/Downloads/chalet.obj";
  if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, path.c_str(), "", true)) {
    throw std::runtime_error(err);
  }
  for (const auto& shape : shapes) {
    for (const auto& index : shape.mesh.indices) {
      quavis::Vertex vertex = {};
      vertex.pos = {
          attrib.vertices[3 * index.vertex_index + 0],
          attrib.vertices[3 * index.vertex_index + 1],
          attrib.vertices[3 * index.vertex_index + 2]
      };
      vertex.color = {
          0.0f,
          0.0f,
          attrib.vertices[3 * index.vertex_index + 2]
      };
      if (vertices_.size() < 10000) {
        vertices_.push_back(vertex);
        indices_.push_back(indices_.size());
      }
    }
  }

  // create logical device with three queues
  quavis::Instance* instance = new quavis::Instance();
  std::shared_ptr<quavis::PhysicalDevice> physicaldevice = std::make_shared<quavis::PhysicalDevice>(instance);
  std::shared_ptr<quavis::LogicalDevice> logicaldevice = std::make_shared<quavis::LogicalDevice>(physicaldevice, 3);
  std::shared_ptr<quavis::Allocator> allocator = std::make_shared<quavis::Allocator>(logicaldevice);

  VkQueue graphics_queue = logicaldevice->queues[0];
  VkQueue compute_queue = logicaldevice->queues[1];
  VkQueue transfer_queue = logicaldevice->queues[2];

  // Create buffers for vertices, indices and uniform buffer object
  std::shared_ptr<quavis::Buffer> vertex_buffer = std::make_shared<quavis::Buffer>(logicaldevice, allocator, sizeof(vertices_[0])*vertices_.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
  std::shared_ptr<quavis::Buffer> index_buffer = std::make_shared<quavis::Buffer>(logicaldevice, allocator, sizeof(indices_[0])*indices_.size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
  std::shared_ptr<quavis::Buffer> uniform_buffer = std::make_shared<quavis::Buffer>(logicaldevice, allocator, 32, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

  // Create shaders
  std::shared_ptr<quavis::Shader> vert_shader = std::make_shared<quavis::Shader>(logicaldevice, VK_SHADER_STAGE_VERTEX_BIT, src_shaders_shader_vert_spv, src_shaders_shader_vert_spv_len);
  std::shared_ptr<quavis::Shader> tesc_shader = std::make_shared<quavis::Shader>(logicaldevice, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, src_shaders_shader_tesc_spv, src_shaders_shader_tesc_spv_len);
  std::shared_ptr<quavis::Shader> tese_shader = std::make_shared<quavis::Shader>(logicaldevice, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, src_shaders_shader_tese_spv, src_shaders_shader_tese_spv_len);
  std::shared_ptr<quavis::Shader> geom_shader = std::make_shared<quavis::Shader>(logicaldevice, VK_SHADER_STAGE_GEOMETRY_BIT, src_shaders_shader_geom_spv, src_shaders_shader_geom_spv_len);
  std::shared_ptr<quavis::Shader> frag_shader = std::make_shared<quavis::Shader>(logicaldevice, VK_SHADER_STAGE_FRAGMENT_BIT, src_shaders_shader_frag_spv, src_shaders_shader_frag_spv_len);
  std::shared_ptr<quavis::Shader> comp_shader = std::make_shared<quavis::Shader>(logicaldevice, VK_SHADER_STAGE_COMPUTE_BIT, src_shaders_shader_comp_spv, src_shaders_shader_comp_spv_len);

  // create color image, depth image and compute shader image
  std::shared_ptr<quavis::Image> color_image = std::make_shared<quavis::Image>(
    logicaldevice,
    allocator,
    graphics_queue,
    width,
    height,
    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
    VK_FORMAT_R8G8B8A8_UNORM,
    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    VK_IMAGE_ASPECT_COLOR_BIT);

  std::shared_ptr<quavis::Image> compute_image = std::make_shared<quavis::Image>(
    logicaldevice,
    allocator,
    graphics_queue,
    width,
    height,
    VK_IMAGE_USAGE_STORAGE_BIT,
    VK_FORMAT_R8G8B8A8_UNORM,
    VK_IMAGE_LAYOUT_GENERAL,
    VK_IMAGE_ASPECT_COLOR_BIT);

  std::shared_ptr<quavis::Image> depth_image = std::make_shared<quavis::Image>(
    logicaldevice,
    allocator,
    graphics_queue,
    width,
    height,
    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
    VK_FORMAT_D32_SFLOAT,
    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    VK_IMAGE_ASPECT_DEPTH_BIT);

  // Upload data to Buffers
  vertex_buffer->SetData((void*)vertices_.data(), transfer_queue);
  index_buffer->SetData((void*)indices_.data(), transfer_queue);
  uniform_buffer->SetData((void*)&uniform_, transfer_queue);
  vkQueueWaitIdle(transfer_queue);

  // Create DescriptorSets
  std::shared_ptr<quavis::DescriptorPool> descriptorpool = std::make_shared<quavis::DescriptorPool>(logicaldevice, 2, 2, 0, 1);
  std::shared_ptr<quavis::DescriptorSet> graphics_descriptorset = std::make_shared<quavis::DescriptorSet>(logicaldevice, descriptorpool, 0,0,1);
  std::shared_ptr<quavis::DescriptorSet> compute_descriptorset = std::make_shared<quavis::DescriptorSet>(logicaldevice, descriptorpool, 2,0,0);

  graphics_descriptorset->AddUniformBuffer(0, uniform_buffer, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_GEOMETRY_BIT);
  compute_descriptorset->AddStorageImage(0, color_image, VK_SHADER_STAGE_COMPUTE_BIT);
  compute_descriptorset->AddStorageImage(1, compute_image, VK_SHADER_STAGE_COMPUTE_BIT);

  graphics_descriptorset->Create();
  compute_descriptorset->Create();

  // Create GraphicsPipeline
  std::vector<std::shared_ptr<quavis::DescriptorSet>> descriptorsets = {graphics_descriptorset};
  std::vector<std::shared_ptr<quavis::Shader>> shaders = {vert_shader, tesc_shader, tese_shader, geom_shader, frag_shader};
  std::shared_ptr<quavis::GraphicsPipeline> gpipe = std::make_shared<quavis::GraphicsPipeline>(
    logicaldevice,
    descriptorsets,
    shaders,
    vertex_buffer,
    index_buffer,
    color_image,
    depth_image
  );

  // Create ComputePipeline
  std::vector<std::shared_ptr<quavis::DescriptorSet>> comp_descriptorsets = {compute_descriptorset};
  std::vector<std::shared_ptr<quavis::Shader>> comp_shaders = {comp_shader};
  std::shared_ptr<quavis::ComputePipeline> cpipe = std::make_shared<quavis::ComputePipeline>(
    logicaldevice,
    comp_descriptorsets,
    comp_shaders,
    width/16,
    height/16,
    1
  );

  // Initialize command buffers

  VkCommandBuffer drawcommand = gpipe->CreateCommandBuffer();
  VkCommandBuffer computecommand = cpipe->CreateCommandBuffer();

  auto t1 = std::chrono::high_resolution_clock::now();
  int N = 1000;
  for (int i = 0; i < N; i++) {
    logicaldevice->SubmitCommandBuffer(graphics_queue, drawcommand, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
    vkQueueWaitIdle(graphics_queue);
  }
  auto t2 = std::chrono::high_resolution_clock::now();
  std::cout << 1.0/(std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1).count()/(float)N/1000.0) << " fps" << std::endl;

  vkDeviceWaitIdle(logicaldevice->vk_handle);

  void* depth = malloc(width*height*4);
  depth = depth_image->GetData(graphics_queue);
  uint8_t image[width * height];
  for (uint32_t i = 0; i < 4 * width * height; i += 4) {
    float px;
    memcpy(&px, (uint8_t*)depth + i, 4);
    image[i/4] = floor((1.0 - px)*255);
  }
  stbi_write_png("bin/depth.png", width, height, 1, (void*)image, 0);
  free(depth);

  void* compute = malloc(width*height*4);
  compute = compute_image->GetData(graphics_queue);
  stbi_write_png("bin/computed.png", width, height, 4, compute, 0);
  free(compute);

  void* rendered = malloc(width*height*4);
  rendered = color_image->GetData(graphics_queue);
  stbi_write_png("bin/renderedd.png", width, height, 4, rendered, 0);
  free(rendered);

  vkDeviceWaitIdle(logicaldevice->vk_handle);
}
