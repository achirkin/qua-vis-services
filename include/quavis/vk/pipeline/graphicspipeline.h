#ifndef QUAVIS_GRAPHICSPIPELINE_H
#define QUAVIS_GRAPHICSPIPELINE_H

#include "quavis/vk/pipeline/pipeline.h"
#include "quavis/vk/device/logicaldevice.h"
#include "quavis/vk/descriptors/descriptorset.h"
#include "quavis/vk/memory/shader.h"
#include "quavis/vk/memory/buffer.h"
#include "quavis/vk/memory/image.h"
#include "quavis/vk/geometry/geoemtry.h"
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
    GraphicsPipeline(LogicalDevice* device,
      std::vector<DescriptorSet*> descriptor_sets,
      std::vector<Shader*> shaders,
      Buffer* vertex_buffer,
      Buffer* index_buffer,
      Image* color_image,
      Image* depth_image);

    /**
     * Destroys the pipeline.
     */
    ~GraphicsPipeline();

    /**
    * Creates the command buffer to run a drawing command in this pipeline.
    */
    VkCommandBuffer CreateCommandBuffer();

  protected:
    VkPipeline InitializePipeline();

  private:
    std::vector<VkPipelineShaderStageCreateInfo> InitializeShaderInfos();
    std::vector<VkAttachmentDescription> InitializeAttachments();
    VkRenderPass InitializeRenderPass();
    VkFramebuffer InitializeFramebuffer();

    Buffer* vertex_buffer_;
    Buffer* index_buffer_;
    Image* color_image_;
    Image* depth_image_;

    VkRenderPass vk_render_pass_;
    VkFramebuffer vk_framebuffer_;
  };
}

#endif
