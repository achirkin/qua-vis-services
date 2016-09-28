#ifndef QUAVIS_INSTANCE_H
#define QUAVIS_INSTANCE_H

namespace Quavis {
  /**
  * A wrapper around the VkInstance structure.
  */
  class Instance {
  public:
    /**
    * Creates a new Vulkan instance and initializes all objects.
    */
    Instance();

    /**
    * Destroys the instance. Note that all dependent objects need to be
    * destroyed beforehand.
    */
    ~Instance();

    /**
    * The handle to the given instance.
    */
    VkInstance vk_handle;

    /**
    * The instance's physical device
    */
    PhysicalDevice physical_device;

    /**
    * The instance's logical device
    */
    LogicalDevice logical_device;

    /**
    * The instance's graphics pipeline
    */
    GraphicsPipeline graphics_pipeline;

    /**
    * The instance's compute pipeline
    */
    ComputePipeline compute_pipeline;
  };
}

#endif
