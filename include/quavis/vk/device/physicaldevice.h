#ifndef QUAVIS_PHYSICALDEVICE_H
#define QUAVIS_PHYSICALDEVICE_H

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
    PhysicalDevice(VkInstance instance);

    /*
    * The vulkan object of the physical device
    */
    VkPhysicalDevice vk_handle;

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
