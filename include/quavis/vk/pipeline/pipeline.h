#ifndef QUAVIS_PIPELINE_H
#define QUAVIS_PIPELINE_H

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
    Pipeline(LogicalDevice device, std::vector<DescriptorSet> descriptor_sets, std::vector<Shader> shaders);

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
    /**
    * Creates the pipeline layout and sets the vk_layout attribute.
    */
    void CreatePipelineLayout();

    /**
    * Create the pipeline and sets the vk_handle attribute.
    */
    virtual void Initialize() = 0;

    LogicalDevice device_;
    std::vector<DescriptorSet> descriptor_sets_;
    std::vector<Shader> shaders_;
  };
}

#endif
