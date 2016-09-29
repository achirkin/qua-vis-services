#include "quavis/vk/device/logicaldevice.h"

#include <iostream>

namespace quavis {

  LogicalDevice::LogicalDevice(PhysicalDevice* physical_device, uint32_t num_queues)
  {
    this->physical_device_ = physical_device;

    // set default features
    this->features_.tessellationShader = VK_TRUE;
    this->features_.geometryShader = VK_TRUE;
    this->features_.fillModeNonSolid = VK_TRUE;

    // get queue families
    uint32_t queue_family = this->GetQueueFamily(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT);

    // create queue generation info
    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    queue_create_infos.push_back(this->GetQueueCreateInfos(queue_family, num_queues));

    // create logical device info
    VkDeviceCreateInfo device_create_info = {
      VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, // sType (see documentation)
      nullptr, // next (see documentation)
      0, // flags (see documentation)
      (uint32_t)queue_create_infos.size(), // number of queue create info objects
      queue_create_infos.data(), // queue meta data
      0, // deprecated & ignored
      nullptr, // depcrecated & ignored
      (uint32_t)this->extensions_.size(), // enabled extensions
      this->extensions_.data(), // extension names
      &this->features_ // enabled device features
    };

    // create logical device
    vkCreateDevice(
      this->physical_device_->vk_handle, // the physical device
      &device_create_info, // logical device metadata
      nullptr, // allocation callback (see documentation)
      &this->vk_handle // the allocated memory for the logical device
    );

    // create queues
    for (uint32_t i = 0; i < num_queues; i++) {
      this->queues.push_back(this->GetQueue(queue_family, i));
    }

    // TODO: Create command pools
  }

  LogicalDevice::~LogicalDevice() {
    vkDeviceWaitIdle(this->vk_handle);
    vkDestroyDevice(this->vk_handle, nullptr);
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

  VkDeviceQueueCreateInfo LogicalDevice::GetQueueCreateInfos(uint32_t queue_family_index, uint32_t num) {
    float priorities[num];
    for (int i = 0; i < num; i++) priorities[i] = 1.0f;

    std::cout << priorities[0] << std::endl;

    VkDeviceQueueCreateInfo queue_create_info;
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = queue_family_index;
    queue_create_info.queueCount = num;
    queue_create_info.pQueuePriorities = priorities;

    return queue_create_info;
  }

  uint32_t LogicalDevice::GetQueueFamily(VkQueueFlags required_flags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT) {
    uint32_t num_queue_families = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(
      this->physical_device_->vk_handle, // the vk device
      &num_queue_families, // the allocated memory for the number of families
      nullptr // the allocated memory for the queue family properties
    );

    // get queues
    std::vector<VkQueueFamilyProperties> queue_families(num_queue_families);
    vkGetPhysicalDeviceQueueFamilyProperties(
      this->physical_device_->vk_handle, // the vk device
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
