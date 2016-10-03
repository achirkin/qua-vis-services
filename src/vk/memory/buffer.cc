#include "quavis/vk/memory/buffer.h"
#include <iostream>

namespace quavis {
  Buffer::Buffer(
    std::shared_ptr<LogicalDevice> logical_device,
    std::shared_ptr<Allocator> allocator,
    uint32_t size,
    VkBufferUsageFlags usage_flags,
    bool staging
  ) {

    this->logical_device_ = logical_device;
    this->allocator_ = allocator;
    this->size = size;
    this->staging_ = staging;

    // Create buffer
    VkBufferCreateInfo buffer_info = {};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = size;
    buffer_info.usage = usage_flags;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    // if staging is enabled, the buffer needs to be usable for transfer
    if (staging) {
      buffer_info.usage = usage_flags | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    }

    vkCreateBuffer(this->logical_device_->vk_handle, &buffer_info, nullptr, &this->vk_handle);

    if (staging) {
      VkBufferCreateInfo staging_buffer_info = {};
      staging_buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
      staging_buffer_info.size = size;
      staging_buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
      staging_buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
      vkCreateBuffer(this->logical_device_->vk_handle, &staging_buffer_info, nullptr, &this->vk_staging_buffer_);
    }

    // Allocate memory
    VkMemoryPropertyFlags buffer_property_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    if (!staging) {
      buffer_property_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    }
    VkMemoryRequirements buffer_req;
    vkGetBufferMemoryRequirements(this->logical_device_->vk_handle, this->vk_handle, &buffer_req);
    this->vk_memory_ = this->allocator_->Allocate(buffer_req, buffer_property_flags);
    vkBindBufferMemory(
      this->logical_device_->vk_handle, // the logical device
      this->vk_handle, // the buffer
      this->vk_memory_, // the buffer memory
      0 // the offset in the memory
    );

    if (staging) {
      VkMemoryPropertyFlags staging_buffer_property_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
      VkMemoryRequirements staging_buffer_req;
      vkGetBufferMemoryRequirements(this->logical_device_->vk_handle, this->vk_staging_buffer_, &staging_buffer_req);
      this->vk_staging_memory_ = this->allocator_->Allocate(staging_buffer_req, staging_buffer_property_flags);
      vkBindBufferMemory(
        this->logical_device_->vk_handle, // the logical device
        this->vk_staging_buffer_, // the buffer
        this->vk_staging_memory_, // the buffer memory
        0 // the offset in the memory
      );
    }
  }

  Buffer::~Buffer() {
    vkDestroyBuffer(this->logical_device_->vk_handle, this->vk_handle, nullptr);
    if (this->staging_) {
        vkDestroyBuffer(this->logical_device_->vk_handle, this->vk_staging_buffer_, nullptr);
      }
  }

  void Buffer::SetData(void* data, VkQueue queue) {
    if (!this->staging_) {
      this->allocator_->SetData(this->vk_memory_, data, this->size);
    }
    else {
      this->allocator_->SetData(this->vk_staging_memory_, data, this->size);

      // define copy region
      VkBufferCopy copyRegion;
      copyRegion.size = this->size;

      // generate command buffer
      VkCommandBuffer command_buffer = this->logical_device_->BeginCommandBuffer(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
      vkCmdCopyBuffer(command_buffer, this->vk_staging_buffer_, this->vk_handle, 1, &copyRegion);
      this->logical_device_->EndCommandBuffer(command_buffer);

      // submit command buffer
      this->logical_device_->SubmitCommandBuffer(queue, command_buffer);
    }
  }

  void* Buffer::GetData(VkQueue queue) {
    if (!this->staging_) {
      return this->allocator_->GetData(this->vk_memory_, this->size);
    }
    else {
      VkBufferCopy copyRegion;
      copyRegion.size = this->size;

      // generate command buffer
      VkCommandBuffer command_buffer = this->logical_device_->BeginCommandBuffer(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
      vkCmdCopyBuffer(command_buffer, this->vk_handle, this->vk_staging_buffer_, 1, &copyRegion);
      this->logical_device_->EndCommandBuffer(command_buffer);

      // submit command buffer
      this->logical_device_->SubmitCommandBuffer(queue, command_buffer);

      return this->allocator_->GetData(this->vk_staging_memory_, this->size);
    }
  }
}
