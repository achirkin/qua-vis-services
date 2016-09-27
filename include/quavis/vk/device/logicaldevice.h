#ifndef QUAVIS_LOGICALDEVICE_H
#define QUAVIS_LOGICALDEVICE_H

#include <vector>

namespace quavis {
  namespace logicaldevice {
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
      * The vulkan handler to the logical device.
      */
      VkDevice device;

      /**
      * The graphics queues
      */
      std::vector<VkQueue> graphics_queues();

      /**
      * The compute queues
      */
      std::vector<VkQueue> compute_queues();

      /**
      * The transfer queues
      */
      std::vector<VkQueue> compute_queues();

    private:
      uint32_t SelectQueueFamily(VkQueueFlags required_flags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT);
      uint32_t RetrieveQueue(uint32_t queue_family_index, uint32_t index);

      // physical device (set in constructor)
      VkPhysicalDevice vk_physical_device_;

      // Default features used for a logical device
      const VkPhysicalDeviceFeatures vk_features_;
      vk_features.tessellationShader = VK_TRUE;
      vk_features.geometryShader = VK_TRUE;
      vk_features.fillModeNonSolid = VK_TRUE;

      // Default extensions
      const std::vector<std::string> vk_logical_device_extensions_();
    };
  }
}

#endif
