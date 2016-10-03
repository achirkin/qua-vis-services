#include "quavis/vk/pipeline/pipeline.h"

namespace quavis {
  Pipeline::Pipeline(std::shared_ptr<LogicalDevice> logical_device, std::vector<std::shared_ptr<DescriptorSet>> descriptor_sets, std::vector<std::shared_ptr<Shader>> shaders) {
    this->logical_device_ = logical_device;
    this->descriptor_sets_ = descriptor_sets;
    this->shaders_ = shaders;

    std::vector<VkDescriptorSetLayout> descriptor_set_layouts;
    for (uint32_t i = 0; i < this->descriptor_sets_.size(); i++) {
      descriptor_set_layouts.push_back(this->descriptor_sets_[i]->vk_layout);
      this->vk_descriptor_sets_.push_back(this->descriptor_sets_[i]->vk_handle);
    }

    // Define Pipeline layout
    VkPipelineLayoutCreateInfo pipeline_layout_info = {
      VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, // sType
      nullptr, // next (see documentation, must be null)
      0, // flags (see documentation, must be 0)
      (uint32_t)descriptor_set_layouts.size(), // layout count
      descriptor_set_layouts.data(), // layouts
      0, // push constant range count
      nullptr // push constant ranges
    };

    // Create pipeline layout
    vkCreatePipelineLayout(
      this->logical_device_->vk_handle,
      &pipeline_layout_info,
      nullptr,
      &this->vk_layout
    );
  }

  Pipeline::~Pipeline() {
    vkDestroyPipelineLayout(this->logical_device_->vk_handle, this->vk_layout, nullptr);
  }
}
