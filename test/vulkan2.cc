#include "quavis/vk/instance.h"
#include "quavis/vk/device/physicaldevice.h"
#include "quavis/vk/device/logicaldevice.h"
#include "quavis/vk/memory/allocator.h"
#include "quavis/vk/memory/buffer.h"
#include "quavis/vk/memory/image.h"
#include "quavis/vk/memory/shader.h"
#include "quavis/vk/descriptors/descriptorpool.h"
#include "quavis/vk/descriptors/descriptorset.h"
#include "quavis/vk/geometry/geoemtry.h"
#include "quavis/shaders.h"

std::vector<quavis::Vertex> vertices_ = {
  //front
  {{-1.0, -1.0,  1.0}, {255,255,255}},
  {{1.0, -1.0,  1.0}, {255,255,255}},
  {{1.0,  1.0,  1.0}, {255,255,255}},
  {{-1.0,  1.0,  1.0}, {255,255,255}},
  // back
  {{-1.0, -1.0, -1.0}, {255,255,255}},
  {{1.0, -1.0, -1.0}, {255,255,255}},
  {{1.0,  1.0, -1.0}, {255,255,255}},
  {{-1.0,  1.0, -1.0}, {255,255,255}}
};

std::vector<uint32_t> indices_ = {
  // front
  0, 1, 2,
  2, 3, 0,
  // top
  1, 5, 6,
  6, 2, 1,
  // back
  7, 6, 5,
  5, 4, 7,
  // bottom
  4, 0, 3,
  3, 7, 4,
  // left
  4, 5, 1,
  1, 0, 4,
  // right
  3, 2, 6,
  6, 7, 3,
};

uint32_t width = 2048;
uint32_t height = 1024;

struct UniformBufferObject {
  quavis::vec3 observation_point;
  float r_max;
  float alpha_max;
};

UniformBufferObject uniform_ = {
  {0,0,0},
  2,
  0.1
};

int main(int argc, char** argv) {
  // create logical device with three queues
  quavis::Instance* instance = new quavis::Instance();
  quavis::PhysicalDevice* physicaldevice = new quavis::PhysicalDevice(instance);
  quavis::LogicalDevice* logicaldevice = new quavis::LogicalDevice(physicaldevice, 3);
  quavis::Allocator* allocator = new quavis::Allocator(logicaldevice);

  VkQueue graphics_queue = logicaldevice->queues[0];
  VkQueue compute_queue = logicaldevice->queues[1];
  VkQueue transfer_queue = logicaldevice->queues[2];

  // Create buffers for vertices, indices and uniform buffer object
  quavis::Buffer* vertex_buffer = new quavis::Buffer(logicaldevice, allocator, sizeof(vertices_[0])*vertices_.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
  quavis::Buffer* index_buffer = new quavis::Buffer(logicaldevice, allocator, sizeof(indices_[0])*indices_.size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
  quavis::Buffer* uniform_buffer = new quavis::Buffer(logicaldevice, allocator, sizeof(UniformBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

  // Create shaders
  quavis::Shader* vert_shader = new quavis::Shader(logicaldevice, VK_SHADER_STAGE_VERTEX_BIT, src_shaders_shader_vert_spv, src_shaders_shader_vert_spv_len);
  quavis::Shader* tesc_shader = new quavis::Shader(logicaldevice, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, src_shaders_shader_tesc_spv, src_shaders_shader_tesc_spv_len);
  quavis::Shader* tese_shader = new quavis::Shader(logicaldevice, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, src_shaders_shader_tese_spv, src_shaders_shader_tese_spv_len);
  quavis::Shader* geom_shader = new quavis::Shader(logicaldevice, VK_SHADER_STAGE_GEOMETRY_BIT, src_shaders_shader_geom_spv, src_shaders_shader_geom_spv_len);
  quavis::Shader* frag_shader = new quavis::Shader(logicaldevice, VK_SHADER_STAGE_FRAGMENT_BIT, src_shaders_shader_frag_spv, src_shaders_shader_frag_spv_len);
  quavis::Shader* comp_shader = new quavis::Shader(logicaldevice, VK_SHADER_STAGE_COMPUTE_BIT, src_shaders_shader_comp_spv, src_shaders_shader_comp_spv_len);

  // create color image, depth image and compute shader image
  quavis::Image* color_image = new quavis::Image(
    logicaldevice,
    allocator,
    transfer_queue,
    width,
    height,
    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
    VK_FORMAT_R8G8B8A8_UNORM,
    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    VK_IMAGE_ASPECT_COLOR_BIT);

  quavis::Image* compute_image = new quavis::Image(
    logicaldevice,
    allocator,
    transfer_queue,
    width,
    height,
    VK_IMAGE_USAGE_STORAGE_BIT,
    VK_FORMAT_R8G8B8A8_UNORM,
    VK_IMAGE_LAYOUT_GENERAL,
    VK_IMAGE_ASPECT_COLOR_BIT);

  quavis::Image* depth_image = new quavis::Image(
    logicaldevice,
    allocator,
    transfer_queue,
    width,
    height,
    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
    VK_FORMAT_D32_SFLOAT,
    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    VK_IMAGE_ASPECT_DEPTH_BIT);

  // Upload data to Buffers
  vertex_buffer->SetData(vertices_.data(), transfer_queue);
  index_buffer->SetData(indices_.data(), transfer_queue);
  uniform_buffer->SetData((void*)&uniform_, transfer_queue);

  // Create DescriptorSets
  quavis::DescriptorPool* descriptorpool = new quavis::DescriptorPool(logicaldevice, 2, 2, 0, 1);
  quavis::DescriptorSet* graphics_descriptorset = new quavis::DescriptorSet(logicaldevice, descriptorpool);
  quavis::DescriptorSet* compute_descriptorset = new quavis::DescriptorSet(logicaldevice, descriptorpool);

  graphics_descriptorset->AddUniformBuffer(uniform_buffer, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_GEOMETRY_BIT);
  compute_descriptorset->AddStorageImage(color_image, VK_SHADER_STAGE_COMPUTE_BIT);
  compute_descriptorset->AddStorageImage(compute_image, VK_SHADER_STAGE_COMPUTE_BIT);

  graphics_descriptorset->Create();
  compute_descriptorset->Create();
}
