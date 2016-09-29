#include "quavis/vk/pipeline/pipeline.h"

namespace quavis {
  Pipeline::Pipeline(LogicalDevice* logical_device, std::vector<DescriptorSet*> descriptor_sets, std::vector<Shader*> shaders) {
    this->logical_device_ = logical_device;
    this->descriptor_sets_ = descriptor_sets;
    this->shaders_ = shaders;

    this->CreatePipelineLayout();
    this->Initialize();
    this->CreateCommandBuffer();
  }

  void Pipeline::CreatePipelineLayout() {
    std::vector<VkDescriptorSetlayout> descriptor_set_layouts;
    for (int i = 0; i < descriptor_sets.size(); i++) {
      descriptor_set_layouts.push_back(descriptor_sets[i]->vk_layout);
    }

    // Define Pipeline layout
    VkPipelineLayoutCreateInfo pipeline_layout_info = {
      VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, // sType
      nullptr, // next (see documentation, must be null)
      0, // flags (see documentation, must be 0)
      descriptor_set_layouts.size(), // layout count
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
}
