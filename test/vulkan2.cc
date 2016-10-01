#include "quavis/vk/instance.h"
#include "quavis/vk/device/physicaldevice.h"
#include "quavis/vk/device/logicaldevice.h"
#include "quavis/vk/memory/allocator.h"
#include "quavis/vk/memory/buffer.h"
#include "quavis/vk/memory/image.h"
#include "quavis/vk/geometry/geoemtry.h"

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

int main(int argc, char** argv) {
  // create logical device with three queues
  quavis::Instance* instance = new quavis::Instance();
  quavis::PhysicalDevice* physicaldevice = new quavis::PhysicalDevice(instance);
  quavis::LogicalDevice* logicaldevice = new quavis::LogicalDevice(physicaldevice, 3);
  quavis::Allocator* allocator = new quavis::Allocator(logicaldevice);

  VkQueue graphics_queue = logicaldevice->queues[0];
  VkQueue compute_queue = logicaldevice->queues[1];
  VkQueue transfer_queue = logicaldevice->queues[2];

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
    
    // create buffers for vertex and index data
    quavis::Buffer* vertex_buffer = new quavis::Buffer(logicaldevice, allocator, sizeof(vertices_[0])*vertices_.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    quavis::Buffer* index_buffer = new quavis::Buffer(logicaldevice, allocator, sizeof(indices_[0])*indices_.size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
}
