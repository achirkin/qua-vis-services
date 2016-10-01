#include "quavis/vk/pipeline/computepipeline.h"

namespace quavis {
  ComputePipeline::ComputePipeline(LogicalDevice* device,
    std::vector<DescriptorSet*> descriptor_sets,
    std::vector<Shader*> shaders,
    uint32_t workgroup_size_1,
    uint32_t workgroup_size_2,
    uint32_t workgroup_size_3) : Pipeline(device, descriptor_sets, shaders) {
      this->vk_handle = this->InitializePipeline();
      this->workgroup_sizes_ = {workgroup_size_1, workgroup_size_2, workgroup_size_3};
    }

  ComputePipeline::~ComputePipeline() {
    vkDestroyPipeline(this->logical_device_->vk_handle, this->vk_handle, nullptr);

  }

  std::vector<VkPipelineShaderStageCreateInfo> ComputePipeline::InitializeShaderInfos() {
    std::vector<VkPipelineShaderStageCreateInfo> shader_infos;
    for(uint32_t i = 0; i < this->shaders_.size(); i++) {
      VkPipelineShaderStageCreateInfo stage_info = {
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, // sType (see documentation)
        nullptr, // next (see documentation, must be null)
        0, // flags (see documentation, must be 0)
        this->shaders_[i]->vk_shader_stage, // stage flag
        this->shaders_[i]->vk_handle, // shader module
        "main", // the pipeline's name
        nullptr // VkSpecializationInfo (see documentation)
      };
      shader_infos.push_back(stage_info);
    }

    return shader_infos;
  }

  VkPipeline ComputePipeline::InitializePipeline() {
    std::vector<VkPipelineShaderStageCreateInfo> shader_infos = this->InitializeShaderInfos();

    // Define pipeline info
    VkComputePipelineCreateInfo pipeline_info = {
      VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO, // sType
      nullptr, // next (see documentation, must be null)
      0, // pipeline create flags (have no child pipelines, so don't care)
      shader_infos[0], // shader stage create infos
      this->vk_layout,
      VK_NULL_HANDLE,
      -1 // parent pipeline index
    };

    VkPipeline pipeline;

    vkCreateComputePipelines(
      this->logical_device_->vk_handle, // logical device
      VK_NULL_HANDLE, // pipeline cache // TODO: Add pipeline cache (?)
      1, // pipeline count
      &pipeline_info, // pipeline infos
      nullptr, // allocation callback
      &pipeline // allocated memory for the pipeline
    );

    return pipeline;
  }

  VkCommandBuffer ComputePipeline::CreateCommandBuffer() {
    VkCommandBuffer command_buffer = this->logical_device_->BeginCommandBuffer(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

    vkCmdBindPipeline(
      command_buffer,
      VK_PIPELINE_BIND_POINT_COMPUTE,
      this->vk_handle
    );

    std::vector<VkDescriptorSet> descriptor_sets;
    for (uint32_t i = 0; i < this->descriptor_sets_.size(); i++) {
      descriptor_sets.push_back(this->descriptor_sets_[i]->vk_handle);
    }

    vkCmdBindDescriptorSets(
      command_buffer,
      VK_PIPELINE_BIND_POINT_COMPUTE,
      this->vk_layout,
      0,
      descriptor_sets.size(),
      descriptor_sets.data(),
      0,
      0
    );

    vkCmdDispatch(
      command_buffer,
      this->workgroup_sizes_[0],
      this->workgroup_sizes_[1],
      this->workgroup_sizes_[2]
    );


    this->logical_device_->EndCommandBuffer(command_buffer);

    return command_buffer;
  }
}
