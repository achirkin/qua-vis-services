#include "quavis/vk/physicaldevice.h"

using namespace quavis::vk;

// TODO: Add extension selection for device, format properties, image format
// properties, queues
PhysicalDevice::PhysicalDevice(VkPhysicalDevice vkPhysicalDevice) {
  this->vkPhysicalDevice_ = vkPhysicalDevice;

  vkGetPhysicalDeviceProperties(
    this->vkPhysicalDevice_,
    &this->vkPhysicalDeviceProperties_
  );

  vkGetPhysicalDeviceFeatures(
    this->vkPhysicalDevice_,
    &this->vkPhysicalDeviceFeatures_
    );

  vkGetPhysicalDeviceMemoryProperties(
    this->vkPhysicalDevice_,
    &this->vkPhysicalDeviceMemoryProperties_
  );
}
