#ifndef QUAVIS_PIPELINE_H
#define QUAVIS_PIPELINE_H

#include "quavis/vk/device/logicaldevice.h"
#include "quavis/vk/descriptors/descriptorset.h"
#include "quavis/vk/memory/shader.h"
#include "quavis/vk/debug.h"

#include <vulkan/vulkan.h>
#include <vector>

namespace quavis {
  /**
  * An abstract class wrapper for VkPipelines objects.
  */
  class Pipeline {
  public:
    /**
    * Creates the pipeline layout using the specified
    * descriptors and shaders. Afterwards the method Initialize() is invoked.
    * It furthermore sets the descriptor_sets and
    * shaders as protected attributes for command buffer creations in a
    * subclass.
    */
    Pipeline(LogicalDevice* logical_device, std::vector<DescriptorSet*> descriptor_sets, std::vector<Shader*> shaders);

    /**
    * Destroys the pipeline object.
    */
    ~Pipeline();

    /**
    * Creates the command buffer to run this pipeline.
    */
    virtual VkCommandBuffer CreateCommandBuffer() = 0;

    /**
    * The vulkan handler to the pipline object
    */
    VkPipeline vk_handle;

    /**
    * The vulkan handler to the pipeline's layout
    */
    VkPipelineLayout vk_layout;

  protected:
    LogicalDevice* logical_device_;
    std::vector<DescriptorSet*> descriptor_sets_;
    std::vector<Shader*> shaders_;

    virtual VkPipeline InitializePipeline() = 0;
  };
}

#endif
