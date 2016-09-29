#include "quavis/vk/memory/image.h"

namespace quavis {
  Image::Image(
    LogicalDevice* logical_device,
    Allocator* allocator,
    VkQueue queue,
    uint32_t width,
    uint32_t height,
    VkImageUsageFlags usage_flags,
    VkFormat format,
    VkImageLayout layout,
    VkImageAspectFlags aspect_flags,
    bool staging) {

    this->logical_device_ = logical_device;
    this->allocator_ = allocator;
    this->width_ = width;
    this->height_ = height;
    this->format_ = format;
    this->aspect_flags_ = aspect_flags;

    // Create image
    VkImageCreateInfo image_info = {
      VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, // sType,
      nullptr, // pNext (see documentation, must be null)
      0, // image flags
      VK_IMAGE_TYPE_2D, // image type
      format, // image format
      {width, height, 1}, // image extent
      1, // level of detail = 1
      1, // layers = 1
      VK_SAMPLE_COUNT_1_BIT, // image sampling per pixel
      VK_IMAGE_TILING_OPTIMAL, // tiling
      usage_flags, // used for transfer
      VK_SHARING_MODE_EXCLUSIVE, // sharing between queue families
      1, // number queue families
      &logical_device->queue_family, // queue family index
      VK_IMAGE_LAYOUT_UNDEFINED // initial layout
    };

    // if staging is enabled, the buffer needs to be usable for transfer
    if (staging) {
      image_info.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    }

    vkCreateImage(this->logical_device_->vk_handle, &image_info, nullptr, &this->vk_handle);

    if (staging) {
      VkImageCreateInfo image_info = {
        VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, // sType,
        nullptr, // pNext (see documentation, must be null)
        0, // image flags
        VK_IMAGE_TYPE_2D, // image type
        format, // image format
        {width, height, 1}, // image extent
        1, // level of detail = 1
        1, // layers = 1
        VK_SAMPLE_COUNT_1_BIT, // image sampling per pixel
        VK_IMAGE_TILING_OPTIMAL, // tiling
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, // used for transfer
        VK_SHARING_MODE_EXCLUSIVE, // sharing between queue families
        1, // number queue families
        &logical_device->queue_family, // queue family index
        VK_IMAGE_LAYOUT_UNDEFINED // initial layout
      };
      vkCreateImage(this->logical_device_->vk_handle, &image_info, nullptr, &this->vk_staging_image_);
    }

    // Allocate memory
    VkMemoryPropertyFlags image_property_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    image_property_flags |= staging && this->staging_property_flags_;
    VkMemoryRequirements image_req;
    vkGetImageMemoryRequirements(this->logical_device_->vk_handle, this->vk_handle, &image_req);
    this->vk_memory_ = this->allocator_->Allocate(image_req, image_property_flags);
    this->memory_size_ = image_req.size;

    vkBindImageMemory(
      this->logical_device_->vk_handle, // the logical device
      this->vk_handle, // the image
      this->vk_memory_, // the image memory
      0 // the offset in the memory
    );

    if (staging) {
      VkMemoryPropertyFlags staging_image_property_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
      staging_image_property_flags |= this->staging_property_flags_;
      VkMemoryRequirements staging_image_req;
      vkGetImageMemoryRequirements(this->logical_device_->vk_handle, this->vk_staging_image_, &staging_image_req);
      this->vk_staging_memory_ = this->allocator_->Allocate(staging_image_req, staging_image_property_flags);
      this->memory_size_ = staging_image_req.size;

      vkBindImageMemory(
        this->logical_device_->vk_handle, // the logical device
        this->vk_staging_image_, // the image
        this->vk_staging_memory_, // the image memory
        0 // the offset in the memory
      );
    }

    this->SetLayout(this->vk_handle, VK_IMAGE_LAYOUT_UNDEFINED, layout, aspect_flags, queue);
    this->layout_ = layout;
    this->SetLayout(this->vk_staging_image_, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, aspect_flags, queue);

    this->vk_view = this->CreateImageView(this->vk_handle, format, aspect_flags);
  }

  Image::~Image() {
    vkDestroyImage(this->logical_device_->vk_handle, this->vk_handle, nullptr);
    if (this->staging_) {
      vkDestroyImage(this->logical_device_->vk_handle, this->vk_staging_image_, nullptr);
    }
  }

  void Image::SetData(void* data, VkQueue queue) {
    if (!this->staging_) {
      this->allocator_->SetData(this->vk_memory_, data, this->memory_size_);
    }
    else {
      this->allocator_->SetData(this->vk_staging_memory_, data, this->memory_size_);

      // define copy region
      VkImageSubresourceLayers subResource = {};
      subResource.aspectMask = this->aspect_flags_;
      subResource.baseArrayLayer = 0;
      subResource.mipLevel = 0;
      subResource.layerCount = 1;

      VkImageCopy copyRegion = {};
      copyRegion.srcSubresource = subResource;
      copyRegion.dstSubresource = subResource;
      copyRegion.srcOffset = {0, 0, 0};
      copyRegion.dstOffset = {0, 0, 0};
      copyRegion.extent.width = this->width_;
      copyRegion.extent.height = this->height_;
      copyRegion.extent.depth = 1;

      // generate command buffer
      VkCommandBuffer command_buffer = this->logical_device_->BeginCommandBuffer(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
      vkCmdCopyImage(command_buffer, this->vk_staging_image_, VK_IMAGE_LAYOUT_GENERAL, this->vk_handle, this->layout_, 1, &copyRegion);
      this->logical_device_->EndCommandBuffer(command_buffer);

      // submit command buffer
      this->logical_device_->SubmitCommandBuffer(queue, command_buffer);
    }
  }

  void* Image::GetData(VkQueue queue) {
    if (!this->staging_) {
      return this->allocator_->GetData(this->vk_memory_, this->memory_size_);
    }
    else {
      // define copy region
      VkImageSubresourceLayers subResource = {};
      subResource.aspectMask = this->aspect_flags_;
      subResource.baseArrayLayer = 0;
      subResource.mipLevel = 0;
      subResource.layerCount = 1;

      VkImageCopy copyRegion = {};
      copyRegion.srcSubresource = subResource;
      copyRegion.dstSubresource = subResource;
      copyRegion.srcOffset = {0, 0, 0};
      copyRegion.dstOffset = {0, 0, 0};
      copyRegion.extent.width = this->width_;
      copyRegion.extent.height = this->height_;
      copyRegion.extent.depth = 1;

      // generate command buffer
      VkCommandBuffer command_buffer = this->logical_device_->BeginCommandBuffer(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
      vkCmdCopyImage(command_buffer, this->vk_handle, this->layout_, this->vk_staging_image_, VK_IMAGE_LAYOUT_GENERAL, 1, &copyRegion);
      this->logical_device_->EndCommandBuffer(command_buffer);

      // submit command buffer
      this->logical_device_->SubmitCommandBuffer(queue, command_buffer);

      return this->allocator_->GetData(this->vk_staging_memory_, this->memory_size_);
    }
  }

  VkImageView Image::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspect_flags) {
    VkImageViewCreateInfo imageview_info = {
      VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, // sType
      nullptr,// pNext (see documentation, must be null)
      0, // flags (see documentation, must be 0)
      image, // the image
      VK_IMAGE_VIEW_TYPE_2D, // the view type
      format, // the format
      {}, // stencil component mapping
      { // VkImageSubresourceRange (what's in the image)
        aspect_flags, // aspect flag
        0, // base level
        1, // level count
        0, // base layer
        1 // layer count
      }
    };

    VkImageView imageview;

    vkCreateImageView(
      this->logical_device_->vk_handle, // the logical device
      &imageview_info, // info
      nullptr, // allocation callback
      &imageview // the allocated memory
    );

    return imageview;
  }

  void Image::SetLayout(VkImage image, VkImageLayout old_layout, VkImageLayout layout, VkImageAspectFlags aspect_flags, VkQueue queue) {
    // normal image
    VkCommandBuffer command_buffer_1 = this->logical_device_->BeginCommandBuffer(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = old_layout;
    barrier.newLayout = layout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = aspect_flags;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    vkCmdPipelineBarrier(
        command_buffer_1,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    this->logical_device_->EndCommandBuffer(command_buffer_1);
    this->logical_device_->SubmitCommandBuffer(queue, command_buffer_1);
  }
}
