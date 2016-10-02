#include "quavis/vk/descriptors/descriptorset.h"
#include <iostream>

namespace quavis {
  DescriptorSet::DescriptorSet(std::shared_ptr<LogicalDevice> device, std::shared_ptr<DescriptorPool> pool, uint32_t num_storage_images, uint32_t num_storage_buffers, uint32_t num_uniform_buffers)
    : buffer_infos_(num_storage_buffers), image_infos_(num_storage_images), uniform_infos_(num_uniform_buffers) {
    this->logical_device_ = device;
    this->descriptor_pool_ = pool;
  }

  DescriptorSet::~DescriptorSet() {
    vkDestroyDescriptorSetLayout(this->logical_device_->vk_handle, this->vk_layout, nullptr);
  }

  uint32_t DescriptorSet::AddStorageImage(uint32_t index, std::shared_ptr<Image> image, VkShaderStageFlags shader_stages) {
    VkDescriptorSetLayoutBinding layout_binding = {};
    layout_binding.binding = this->layout_bindings_.size();
    layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    layout_binding.descriptorCount = 1;
    layout_binding.stageFlags = shader_stages;

    VkDescriptorImageInfo descriptor_info = {};
    descriptor_info.sampler = VK_NULL_HANDLE;
    descriptor_info.imageView = image->vk_view;
    descriptor_info.imageLayout = image->vk_layout;

    this->image_infos_[index] = descriptor_info;

    VkWriteDescriptorSet write_set = {};
    write_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write_set.dstSet = this->vk_handle;
    write_set.dstBinding = this->layout_bindings_.size();
    write_set.dstArrayElement = 0;
    write_set.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    write_set.descriptorCount = 1;
    write_set.pImageInfo = &this->image_infos_[index];
    write_set.pBufferInfo = nullptr;
    write_set.pTexelBufferView = nullptr;

    this->layout_bindings_.push_back(layout_binding);
    this->write_sets_.push_back(write_set);

    return this->layout_bindings_.size() - 1;
  }

  uint32_t DescriptorSet::AddUniformBuffer(uint32_t index, std::shared_ptr<Buffer> buffer, VkShaderStageFlags shader_stages) {
    VkDescriptorSetLayoutBinding layout_binding = {};
    layout_binding.binding = this->layout_bindings_.size();
    layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layout_binding.descriptorCount = 1;
    layout_binding.stageFlags = shader_stages;

    VkDescriptorBufferInfo descriptor_info = {};
    descriptor_info.buffer = buffer->vk_handle;
    descriptor_info.offset = 0;
    descriptor_info.range = buffer->size;

    this->uniform_infos_[index] = descriptor_info;

    VkWriteDescriptorSet write_set = {};
    write_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write_set.dstSet = this->vk_handle;
    write_set.dstBinding = this->layout_bindings_.size();
    write_set.dstArrayElement = 0;
    write_set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write_set.descriptorCount = 1;
    write_set.pImageInfo = nullptr;
    write_set.pBufferInfo = &this->uniform_infos_[index];
    write_set.pTexelBufferView = nullptr;

    this->layout_bindings_.push_back(layout_binding);
    this->write_sets_.push_back(write_set);

    return this->layout_bindings_.size() - 1;
  }

  uint32_t DescriptorSet::AddStorageBuffer(uint32_t index, std::shared_ptr<Buffer> buffer, VkShaderStageFlags shader_stages) {
    VkDescriptorSetLayoutBinding layout_binding = {};
    layout_binding.binding = this->layout_bindings_.size();
    layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    layout_binding.descriptorCount = 1;
    layout_binding.stageFlags = shader_stages;

    VkDescriptorBufferInfo descriptor_info = {};
    descriptor_info.buffer = buffer->vk_handle;
    descriptor_info.offset = 0;
    descriptor_info.range = buffer->size;

    this->buffer_infos_[index] = descriptor_info;

    VkWriteDescriptorSet write_set = {};
    write_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write_set.dstSet = this->vk_handle;
    write_set.dstBinding = this->layout_bindings_.size();
    write_set.dstArrayElement = 0;
    write_set.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    write_set.descriptorCount = 1;
    write_set.pImageInfo = nullptr;
    write_set.pBufferInfo = &this->buffer_infos_[index];
    write_set.pTexelBufferView = nullptr;

    this->layout_bindings_.push_back(layout_binding);
    this->write_sets_.push_back(write_set);

    return this->layout_bindings_.size() - 1;
  }

  void DescriptorSet::Create() {
    VkDescriptorSetLayoutCreateInfo layout_info = {};
    layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_info.flags = 0;
    layout_info.bindingCount = this->layout_bindings_.size();
    layout_info.pBindings = this->layout_bindings_.data();

    vkCreateDescriptorSetLayout(
      this->logical_device_->vk_handle,
      &layout_info,
      nullptr,
      &this->vk_layout
    );

    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = this->descriptor_pool_->vk_handle;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &this->vk_layout;
    vkAllocateDescriptorSets(this->logical_device_->vk_handle, &allocInfo, &this->vk_handle);

    for (uint32_t i = 0; i < this->layout_bindings_.size(); i++) {
      this->write_sets_[i].dstSet = this->vk_handle;
      this->Update(i);
    }

  }

  void DescriptorSet::Update(uint32_t index) {
    vkUpdateDescriptorSets(
      this->logical_device_->vk_handle,
      1,
      &this->write_sets_[index],
      0,
      nullptr
    );
  }
}
