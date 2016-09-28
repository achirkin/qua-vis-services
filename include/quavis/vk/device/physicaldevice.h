#ifndef QUAVIS_PHYSICALDEVICE_H
#define QUAVIS_PHYSICALDEVICE_H

#include "quavis/vk/instance.h"
#include "quavis/vk/debug.h"

#include <vulkan/vulkan.h>

namespace quavis {
  /**
  * The PhysicalDevice class is a wrapper around the VkPhysicalDevice struct.
  * It provides convencience methods that ease instantiation of a vulkan
  * instance and, in this case, automatically chooses a physical device.
  */
  class PhysicalDevice {
  public:
    /**
    * Finds and picks an appropriate physical device that supports the
    * specified queue types, extensions and layers.
    */
    PhysicalDevice(Instance instance);

    /**
    * Checks for a physical device whether it supports a given extension
    */
    static bool hasExtension(VkPhysicalDevice device, const char* extension_name);

    /**
    * Checks for a physical device whether it supports a given layer
    */
    static bool hasLayer(VkPhysicalDevice device, const char* layer_name);

    /**
    * Checks for a physical device whether it supports a given queue type
    */
    static bool hasQueueType(VkPhysicalDevice device, VkQueueFlags queue_flags);

    /*
    * The vulkan object of the physical device
    */
    VkPhysicalDevice vk_handle;

  private:
    const std::vector<const char*> extensions_ = {
      "VK_EXT_debug_report"
    };

    const std::vector<const char*> layers_ = {
      "VK_LAYER_LUNARG_standard_validation"
    };
  };
}

#endif
