#ifndef QUAVIS_LOGICALDEVICE_H
#define QUAVIS_LOGICALDEVICE_H

#include <vector>

namespace quavis {
  /**
  * The LogicalDevice class is a wrapper around the VkDevice struct. It
  * provides methods for easy queue initialization and management as well as
  * command pool initialization.
  */
  class LogicalDevice {
  public:
    /**
    * Creates a new logical device on the selected physical device.
    * Optionally, the number of graphics- and compute queues can be
    * specfified.
    *
    * The method selects 2 (not necessarily different) queue families
    * from the specified device.
    * The graphics queues are guaranteed to be usable for graphics while
    * the compute queues are usable for compute shaders and
    * the transfer queues are usable for memory transfer.
    */
    LogicalDevice(VkPhysicalDevice physical_device, uint32_t graphics_queues = 1, uint32_t compute_queues = 1, uint32_t transfer_queues = 1);

    /**
    * Destroys the logical device safely. Note that all objects that are
    * dependant on the logical device need to be destroyed beforehand.
    */
    ~LogicalDevice();

    /**
    * Returns the command pool of the given queue
    */
    CommandPool GetCommandPool(VkQueue queue);

    /**
    * The vulkan handler to the logical device.
    */
    VkDevice vk_handle;

    /**
    * The graphics queues (used for render pass)
    */
    std::vector<VkQueue> graphics_queues();

    /**
    * The compute queues (used for compute shader)
    */
    std::vector<VkQueue> compute_queues();

    /**
    * The transfer queues (used for memory transfer)
    */
    std::vector<VkQueue> compute_queues();

    /**
    * The corresponding physical device
    */
    VkPhysicalDevice physical_device_;

  private:
    uint32_t GetQueueFamily(VkQueueFlags required_flags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT);
    uint32_t GetQueue(uint32_t queue_family_index, uint32_t queue_index);
    void CreateCommandPool(uint32_t queue_family_index);


    // Default features used for a logical device
    VkPhysicalDeviceFeatures vk_features_;
    vk_features.tessellationShader = VK_TRUE;
    vk_features.geometryShader = VK_TRUE;
    vk_features.fillModeNonSolid = VK_TRUE;

    // Default extensions
    const std::vector<const char*> extensions_();
  };
}

#endif
