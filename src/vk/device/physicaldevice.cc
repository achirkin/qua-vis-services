#include "quavis/vk/device/physicaldevice.h"

namespace quavis {
  PhysicalDevice::PhysicalDevice(Instance* instance) {
    // check how many devices there are
    uint32_t num_devices = 0;
    vkEnumeratePhysicalDevices(
      instance->vk_handle, // the vk instance
      &num_devices, // The allocated memory for the number of devices
      nullptr // the allocated memory for the devices itself.
    );
    if (num_devices == 0)
      throw "No suitible device.";

    // get the devices
    std::vector<VkPhysicalDevice> devices(num_devices);
    vkEnumeratePhysicalDevices(
      instance->vk_handle, // the vk instance
      &num_devices, // The allocated memory for the number of devices
      devices.data() // the allocated memory for the devices itself.
    );

    // For requirement checking we add a set from which non-suitible devices are
    // being removed on the fly
    std::set<VkPhysicalDevice> devices_set(devices.begin(), devices.end());

    // Remove all devices that do not have graphics or compute queue family
    // TODO: Check different device properties (features, extensions, ...)
    std::set<VkPhysicalDevice>::iterator device_it;
    for (device_it = devices_set.begin(); device_it != devices_set.end();) {

      /////////////////// BEGIN CHECK QUEUE FAMILIES
      bool has_graphics_queue = false;
      bool has_compute_queue = false;
      bool has_transfer_queue = false;

      uint32_t num_queue_families = 0;
      vkGetPhysicalDeviceQueueFamilyProperties(
        *device_it, // the vk device
        &num_queue_families, // the allocated memory for the number of families
        nullptr // the allocated memory for the queue family properties
      );

      // No queues
      if (num_queue_families == 0) {
        device_it = devices_set.erase(device_it); // no queue families
        continue;
      }

      // get queues
      std::vector<VkQueueFamilyProperties> queue_families(num_queue_families);
      vkGetPhysicalDeviceQueueFamilyProperties(
        *device_it, // the vk device
        &num_queue_families, // the allocated memory for the number of families
        queue_families.data() // the allocated memory for the queue family properties
      );

      for (VkQueueFamilyProperties queue_family : queue_families) {
        if (!has_graphics_queue)
          has_graphics_queue = queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT;
        if (!has_compute_queue)
          has_compute_queue = queue_family.queueFlags & VK_QUEUE_COMPUTE_BIT;
        if (!has_transfer_queue)
          has_transfer_queue = queue_family.queueFlags & VK_QUEUE_TRANSFER_BIT;
      }

      if (!has_graphics_queue || !has_compute_queue || !has_transfer_queue) {
        device_it = devices_set.erase(device_it);
        continue;
      }

      device_it++;
    }

    // Check if there are devices meeting the requirements
    // if so, pick the first one (random)
    if (devices_set.size() == 0)
      throw "No suitible device";
    else
      this->vk_handle = *devices_set.begin();
  }
}
