#include "quavis/vk/device/logicaldevice.h"

namespace quavis {

  LogicalDevice::LogicalDevice(PhysicalDevice* physical_device,
    uint32_t num_graphics_queues,
    uint32_t num_compute_queues,
    uint32_t num_transfer_queues)
  {
    this->physical_device_ = physical_device;

    // get queue families
    uint32_t graphics_queue_family = this->GetQueueFamily(VK_QUEUE_GRAPHICS_BIT);
    uint32_t compute_queue_family = this->GetQueueFamily(VK_QUEUE_COMPUTE_BIT);
    uint32_t transfer_queue_family = this->GetQueueFamily(VK_QUEUE_TRANSFER_BIT);

    // create queue generation info
    std::vector<VkDeviceQueueCreateInfo> queue_create_infos();
    queue_create_infos.push_back(this->GetQueueCreateInfos(graphics_queue_family, num_graphics_queues));
    queue_create_infos.push_back(this->GetQueueCreateInfos(compute_queue_family, num_compute_queues));
    queue_create_infos.push_back(this->GetQueueCreateInfos(transfer_queue_family, num_transfer_queues));

    // create logical device info
    VkDeviceCreateInfo device_create_info = {
      VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, // sType (see documentation)
      nullptr, // next (see documentation)
      0, // flags (see documentation)
      queue_create_infos.size(), // number of queue create info objects
      &queue_create_infos.data(), // queue meta data
      0, // deprecated & ignored
      nullptr, // depcrecated & ignored
      this->extensions_.size(), // enabled extensions
      this->extensions_.data(), // extension names
      &this->features_ // enabled device features
    };

    // create logical device
    debug::handleVkResult(
      vkCreateDevice(
        this->physical_device_->vk_handle, // the physical device
        &device_create_info, // logical device metadata
        nullptr, // allocation callback (see documentation)
        &this->vk_handle // the allocated memory for the logical device
      )
    );

    // create queues
    for (uint32_t i = 0; i < num_graphics_queues; i++) {
      this->graphics_queues.push_back(this->GetQueue(graphics_queue_family, i));
    }

    for (uint32_t i = 0; i < num_compute_queues; i++) {
      this->compute_queues.push_back(this->GetQueue(compute_queue_family, i));
    }

    for (uint32_t i = 0; i < num_transfer_queues; i++) {
      this->transfer_queues.push_back(this->GetQueue(transfer_queue_family, i));
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
    std::vector<float> priorities(num, 1.0f);

    VkDeviceQueueCreateInfo queue_create_info;
    queue_create_info.queueFamilyIndex = queue_family_index;
    queue_create_info.queueCount = num;
    queue_create_info.pQueuePriorities = priorities.data();

    return queue_create_info;
  }

  uint32_t LogicalDevice::GetQueueFamily(VkQueueFlags required_flags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT) {
    uint32_t num_queue_families = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(
      this->physical_device->vk_handle, // the vk device
      &num_queue_families, // the allocated memory for the number of families
      nullptr // the allocated memory for the queue family properties
    );

    // get queues
    std::vector<VkQueueFamilyProperties> queue_families(num_queue_families);
    vkGetPhysicalDeviceQueueFamilyProperties(
      this->physical_device->vk_handle, // the vk device
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
