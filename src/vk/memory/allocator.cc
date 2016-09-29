#include "quavis/vk/memory/allocator.h"

namespace quavis {
  Allocator::Allocator(LogicalDevice* logical_device) {
    this->logical_device_ = logical_device;
  }

  Allocator::~Allocator() {
    for (int i = this->allocated_memory_.size() - 1; i >= 0; i++) {
      vkFreeMemory(this->logical_device_->vk_handle, this->allocated_memory_[i], nullptr);
    }
  }

  VkDeviceMemory Allocator::Allocate(VkMemoryRequirements memory_requirements, VkMemoryPropertyFlags flags) {
    // get memory types
    VkPhysicalDeviceMemoryProperties physical_device_memory_properties;
    vkGetPhysicalDeviceMemoryProperties(
      this->logical_device_->physical_device->vk_handle,
      &physical_device_memory_properties);

    // find correct memory type
    uint32_t memory_type = 0;
    uint32_t memory_type_bits = memory_requirements.memoryTypeBits;
    for (uint32_t i = 0; i < physical_device_memory_properties.memoryTypeCount; i++) {
      if ((memory_type_bits & (1 << i)) && (physical_device_memory_properties.memoryTypes[i].propertyFlags & flags) == flags) {
        memory_type = i;
        break;
      }
    }

    // allocate memory
    VkDeviceMemory memory;

    VkMemoryAllocateInfo allocation_info = {
      VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, // sType
      nullptr, // pNext (see documentation, must be null)
      memory_requirements.size, // the memory size
      memory_type // the memory type index
    };

    vkAllocateMemory(
      this->logical_device_->vk_handle, // the logical devcie
      &allocation_info, // the allocation info
      nullptr, // allocation callback
      &memory // allocated memory for memory object
    );

    // store memory and return it
    this->allocated_memory_.push_back(memory);
    return memory;
  }

  void Allocator::SetData(VkDeviceMemory destination_memory, void* data, uint32_t size) {
    void* gpu_buffer;

    vkMapMemory(
      this->logical_device_->vk_handle,
      destination_memory,
      0,
      size,
      0,
      &gpu_buffer);

    memcpy(gpu_buffer, data, (size_t)size);

    vkUnmapMemory(
      this->logical_device_->vk_handle,
      destination_memory
    );
  }

  void* Allocator::GetData(VkDeviceMemory source_memory, uint32_t size) {
    void* gpu_buffer;
    void* data = malloc(size);

    vkMapMemory(
      this->logical_device_->vk_handle,
      source_memory,
      0,
      size,
      0,
      &gpu_buffer);

    memcpy(gpu_buffer, data, (size_t)size);

    vkUnmapMemory(
      this->logical_device_->vk_handle,
      source_memory
    );

    return data;
  }
}
