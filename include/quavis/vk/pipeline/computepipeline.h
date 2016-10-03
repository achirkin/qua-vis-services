#ifndef QUAVIS_COMPUTEPIPELINE_H
#define QUAVIS_COMPUTEPIPELINE_H

#include "quavis/vk/pipeline/pipeline.h"
#include "quavis/vk/device/logicaldevice.h"
#include "quavis/vk/descriptors/descriptorset.h"
#include "quavis/vk/memory/shader.h"
#include "quavis/vk/debug.h"

#include <vulkan/vulkan.h>
#include <vector>

namespace quavis {
  /**
  * A wrapper around a graphics pipeline for indexed vertex drawings with
  * arbitrary shaders.
  */
  class ComputePipeline : Pipeline {
  public:
    /**
    * Calls the super class constructor.
    */
    ComputePipeline(LogicalDevice* device,
      std::vector<DescriptorSet*> descriptor_sets,
      std::vector<Shader*> shaders,
      uint32_t workgroup_size_1 = 1,
      uint32_t workgroup_size_2 = 1,
      uint32_t workgroup_size_3 = 1);

    /**
    * Destroys the pipeline.
    */
    ~ComputePipeline();

    /**
    * Creates a command buffer for this pipeline.
    */
    VkCommandBuffer CreateCommandBuffer();

  protected:
    VkPipeline InitializePipeline();

  private:
    std::vector<VkPipelineShaderStageCreateInfo> InitializeShaderInfos();
    std::vector<uint32_t> workgroup_sizes_;
  };
}

#endif
