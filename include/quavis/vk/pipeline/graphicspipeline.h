#ifndef QUAVIS_GRAPHICSPIPELINE_H
#define QUAVIS_GRAPHICSPIPELINE_H

#include "quavis/vk/pipeline/pipeline.h"
#include "quavis/vk/device/logicaldevice.h"
#include "quavis/vk/descriptors/descriptorset.h"
#include "quavis/vk/memory/shader.h"
#include "quavis/vk/memory/buffer.h"
#include "quavis/vk/debug.h"

#include <vulkan/vulkan.h>
#include <vector>

namespace quavis {
  /**
  * A wrapper around a graphics pipeline for indexed vertex drawings with
  * arbitrary shaders.
  */
  class GraphicsPipeline : Pipeline {
  public:
    /**
    * Calls the superclass constructor and stores the necessary buffers.
    */
    GraphicsPipeline(LogicalDevice device,
      std::vector<DescriptorSet> descriptor_sets,
      std::vector<Shader> shaders,
      Buffer vertex_buffer,
      Buffer index_buffer) : Pipeline(device, descriptor_sets, shaders);

    /**
    * Destroys the pipeline object.
    */
    ~GraphicsPipeline() : ~Pipeline();

    /**
    * Creates the command buffer to run a drawing command in this pipeline.
    */
    VkCommandBuffer CreateCommandBuffer();

  protected:
    /**
    * Creates the graphics pipeline object using the specified shaders
    * and sets the vk_handle object.
    */
    void Initialize();

  private:
    Buffer vertex_buffer_;
    Buffer index_buffer_;
  };
}

#endif
