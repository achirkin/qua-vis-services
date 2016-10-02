#include "quavis/vk/descriptors/descriptorpool.h"

namespace quavis {
  DescriptorPool::DescriptorPool(std::shared_ptr<LogicalDevice> logical_device, uint32_t num_sets, uint32_t num_storage_images, uint32_t num_storage_buffers, uint32_t num_uniform_buffers) {
    this->logical_device_ = logical_device;

    VkDescriptorPoolSize uniforms = {};
    uniforms.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uniforms.descriptorCount = num_uniform_buffers;

    VkDescriptorPoolSize buffers = {};
    buffers.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    buffers.descriptorCount = num_storage_buffers;

    VkDescriptorPoolSize images = {};
    images.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE; //TODO: COMPUTESHADERTODO
    images.descriptorCount = num_storage_images;

    // create pool
    std::vector<VkDescriptorPoolSize> poolSizes = {
      uniforms,
      buffers,
      images
    };

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = poolSizes.size();
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = 2;

    vkCreateDescriptorPool(this->logical_device_->vk_handle, &poolInfo, nullptr, &this->vk_handle);
  }

  DescriptorPool::~DescriptorPool() {
    vkDestroyDescriptorPool(this->logical_device_->vk_handle, this->vk_handle, nullptr);
  }
}
