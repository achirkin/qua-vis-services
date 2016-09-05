#ifndef QUAVIS_PHYSICALDEVICE_H
#define QUAVIS_PHYSICALDEVICE_H

#include "quavis/vk/result.h"

#include <vulkan/vulkan.h>

namespace quavis {
  namespace vk {
    /**
    * The PhysicalDevice class implements some functions for checking the
    * suitibility of the device within
    */
    class PhysicalDevice {
    public:
      PhysicalDevice(VkPhysicalDevice vkPhysicalDevice);
      ~PhysicalDevice();

    private:
      VkPhysicalDevice vkPhysicalDevice_ = VK_NULL_HANDLE;
      VkPhysicalDeviceProperties vkPhysicalDeviceProperties_;

      // descriptors
      VkPhysicalDeviceFeatures vkPhysicalDeviceFeatures_;
      VkPhysicalDeviceMemoryProperties vkPhysicalDeviceMemoryProperties_;
      VkFormatProperties vkFormatProperties_;
      VkImageFormatProperties vkImageFormatProperties_;
      VkQueueFamilyProperties vkQueueFamilyProperties_;
    };
  }
}

#endif
