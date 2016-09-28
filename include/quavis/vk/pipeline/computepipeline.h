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
    ComputePipeline(LogicalDevice device,
      std::vector<DescriptorSet> descriptor_sets,
      std::vector<Shader> shaders) : Pipeline(device, descriptor_sets, shaders);

    /**
    * Destroys the pipeline.
    */
    ~ComputePipeline();

    /**
    * Creates a command buffer for this pipeline.
    */
    VkCommandBuffer CreateCommandBuffer();

  protected:
    /**
    * Creates the graphics pipeline object using the specified shaders
    * and sets the vk_handle object.
    */
    void Initialize();
  };
}

#endif
