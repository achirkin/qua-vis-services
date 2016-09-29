#ifndef QUAVIS_PHYSICALDEVICE_H
#define QUAVIS_PHYSICALDEVICE_H

#include "quavis/vk/instance.h"
#include "quavis/vk/debug.h"

#include <vulkan/vulkan.h>
#include <set>

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
    PhysicalDevice(Instance* instance);

    /*
    * The vulkan object of the physical device
    */
    VkPhysicalDevice vk_handle;
  };
}

#endif
