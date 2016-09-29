#include "quavis/vk/descriptors/descriptorset.h"

namespace quavis {
  DescriptorSet::DescriptorSet(LogicalDevice* device, DescriptorPool* pool) {
    this->logical_device_ = device;
    this->descriptor_pool_ = pool;
  }

  uint32_t DescriptorSet::AddStorageImage(Image* image, VkShaderStageFlags shader_stages) {
    VkDescriptorSetLayoutBinding layout_binding;
    layout_binding.binding = this->layout_bindings_.size();
    layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    layout_binding.descriptorCount = 1;
    layout_binding.stageFlags = shader_stages;

    VkDescriptorImageInfo descriptor_info = {
      VK_NULL_HANDLE,
      image->vk_view,
      image->vk_layout
    };

    VkWriteDescriptorSet write_set = {};
    write_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write_set.dstSet = this->vk_handle;
    write_set.dstBinding = this->layout_bindings_.size();
    write_set.dstArrayElement = 0;
    write_set.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    write_set.descriptorCount = 1;
    write_set.pImageInfo = &descriptor_info;
    write_set.pBufferInfo = nullptr;
    write_set.pTexelBufferView = nullptr;

    this->layout_bindings_.push_back(layout_binding);
    this->write_sets_.push_back(write_set);

    return this->layout_bindings_.size() - 1;
  }

  uint32_t DescriptorSet::AddUniformBuffer(Buffer* buffer, VkShaderStageFlags shader_stages) {
    VkDescriptorSetLayoutBinding layout_binding;
    layout_binding.binding = this->layout_bindings_.size();
    layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layout_binding.descriptorCount = 1;
    layout_binding.stageFlags = shader_stages;

    VkDescriptorBufferInfo descriptor_info;
    descriptor_info.buffer = buffer->vk_handle;
    descriptor_info.offset = 0;
    descriptor_info.range = buffer->size;

    VkWriteDescriptorSet write_set = {};
    write_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write_set.dstSet = this->vk_handle;
    write_set.dstBinding = this->layout_bindings_.size();
    write_set.dstArrayElement = 0;
    write_set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write_set.descriptorCount = 1;
    write_set.pImageInfo = nullptr;
    write_set.pBufferInfo = &descriptor_info;
    write_set.pTexelBufferView = nullptr;

    this->layout_bindings_.push_back(layout_binding);
    this->write_sets_.push_back(write_set);

    return this->layout_bindings_.size() - 1;
  }

  uint32_t DescriptorSet::AddStorageBuffer(Buffer* buffer, VkShaderStageFlags shader_stages) {
    VkDescriptorSetLayoutBinding layout_binding;
    layout_binding.binding = this->layout_bindings_.size();
    layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    layout_binding.descriptorCount = 1;
    layout_binding.stageFlags = shader_stages;

    VkDescriptorBufferInfo descriptor_info;
    descriptor_info.buffer = buffer->vk_handle;
    descriptor_info.offset = 0;
    descriptor_info.range = buffer->size;

    VkWriteDescriptorSet write_set = {};
    write_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write_set.dstSet = this->vk_handle;
    write_set.dstBinding = this->layout_bindings_.size();
    write_set.dstArrayElement = 0;
    write_set.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    write_set.descriptorCount = 1;
    write_set.pImageInfo = nullptr;
    write_set.pBufferInfo = &descriptor_info;
    write_set.pTexelBufferView = nullptr;

    this->layout_bindings_.push_back(layout_binding);
    this->write_sets_.push_back(write_set);

    return this->layout_bindings_.size() - 1;
  }

  void DescriptorSet::Create() {
    VkDescriptorSetLayoutCreateInfo layout_info = {};
    layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
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
      this->Update(i);
    }
  }

  void DescriptorSet::Update(uint32_t index) {
    vkUpdateDescriptorSets(
      this->logical_device_->vk_handle,
      this->write_sets_.size(),
      this->write_sets_.data(),
      0,
      nullptr
    );
  }
}
