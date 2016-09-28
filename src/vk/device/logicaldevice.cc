#include "quavis/vk/device/logicaldevice.h"

namespace quavis {

  LogicalDevice::LogicalDevice(PhysicalDevice* physical_device,
    uint32_t graphics_queues,
    uint32_t compute_queues,
    uint32_t transfer_queues)
  {
    this->PhysicaDevice = physical_device;
    // TODO
  }

  LogicalDevice::~LogicalDevice() {
    // TODO
  }

  VkCommandBuffer LogicalDevice::BeginCommandBuffer(VkQueue queue,
    VkCommandBufferUsageFlags flags) {
    // TODO
  }

  void LogicalDevice::EndCommandBuffer(VkCommandBuffer command_buffer) {
    // TODO
  }

  void LogicalDevice::SubmitCommandBuffer(VkQueue queue, VkCommandBuffer command_buffer) {
    // TODO
  }

  void LogicalDevice::CreateCommandPool(uint32_t queue_family_index) {
    // TODO
  }

  CommandPool LogicalDevice::GetCommandPool(uint32_t queue_family_index) {
    // TODO
  }

  uint32_t LogicalDevice::GetQueueFamily(VkQueueFlags required_flags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT) {
    uint32_t num_queue_families = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(
      this->vk_physical_device_, // the vk device
      &num_queue_families, // the allocated memory for the number of families
      nullptr // the allocated memory for the queue family properties
    );

    // get queues
    std::vector<VkQueueFamilyProperties> queue_families(num_queue_families);
    vkGetPhysicalDeviceQueueFamilyProperties(
      this->vk_physical_device_, // the vk device
      &num_queue_families, // the allocated memory for the number of families
      queue_families.data() // the allocated memory for the queue family properties
    );

    // get index of suitible queue family
    uint32_t queue_family_index = 0;
    for (VkQueueFamilyProperties queue_family : queue_families) {
      if (queue_family.queueFlags & required_flags)
        break;
      queue_family_index++;
    }

    return queue_family_index;
  }

  VkQueue LogicalDevice::GetQueue(uint32_t queue_family_index, uint32_t queue_index) {
    VkQueue queue;

    vkGetDeviceQueue(
      this->vk_handle, // the logical device
      queue_family_index, // the queue family from which we want the queue
      queue_index, // the index of the queue we want < NUM_QUEUES_IN_FAMILY
      &queue // the allocated memory for the queue
    );

    return queue;
  }
}
