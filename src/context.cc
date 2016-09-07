#include "quavis/context.h"

using namespace quavis;

Context::Context() {
  this->InitializeVkInstance();
  this->InitializeVkPhysicalDevice();
  this->InitializeVkLogicalDevice();
  this->InitializeVkPipeline();
  this->InitializeVkSwapChain();
}

Context::~Context() {
  vkDestroyInstance(this->vk_instance_, nullptr);
}

void Context::InitializeVkInstance() {
  uint32_t version = VK_MAKE_VERSION(VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);

  VkApplicationInfo vkApplicationInfo = VkApplicationInfo {
    VK_STRUCTURE_TYPE_APPLICATION_INFO, // type (see documentation)
    nullptr, // next structure (see documentation)
    "Quavis", // application name
    version, // quavis version
    "Quavis", // engine name
    version, // engine version,
    VK_API_VERSION_1_0 // vk version
  };

  // TODO: Check if debug during instance creation and add glfw extension
  VkInstanceCreateInfo vkInstanceCreateInfo = VkInstanceCreateInfo {
    VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, // type (see documentation)
    nullptr, // next structure (see documentation)
    0, // flags, reserver by vulkan api for future api versions
    &vkApplicationInfo, // application info (see aboive)
    0, // number of layers (atm no debug/validation layers used)
    nullptr, // layer names
    0, // number of extensions (atm no extensions used, here could be glfw)
    nullptr // extension names
  };

  // create the instance object
  vk::handleVkResult(
    vkCreateInstance(
      &vkInstanceCreateInfo, // creation info (see above)
      nullptr, // allocation handler (gives specific info on memory locations)
      &this->vk_instance_ // pointer where to store the instance
    )
  );
}

void Context::InitializeVkPhysicalDevice() {
  // check how many devices there are
  uint32_t num_devices = 0;
  vkEnumeratePhysicalDevices(
    this->vk_instance_, // the vk instance
    &num_devices, // The allocated memory for the number of devices
    nullptr // the allocated memory for the devices itself.
  );
  if (num_devices == 0)
    throw "No suitible device.";

  // get the devices
  std::vector<VkPhysicalDevice> devices(num_devices);
  vkEnumeratePhysicalDevices(
    this->vk_instance_, // the vk instance
    &num_devices, // The allocated memory for the number of devices
    devices.data() // the allocated memory for the devices itself.
  );

  // For requirement checking we add a set from which non-suitible devices are
  // being removed on the fly
  std::set<VkPhysicalDevice> devices_set(devices.begin(), devices.end());

  // Remove all devices that do not have graphics or compute queue family
  std::set<VkPhysicalDevice>::iterator device_it;
  for (device_it = devices_set.begin(); device_it != devices_set.end();) {
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

    bool has_graphics_queue = false;
    bool has_compute_queue = false;
    for (VkQueueFamilyProperties queue_family : queue_families) {
      if (!has_graphics_queue)
        has_graphics_queue = queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT;

      if (!has_compute_queue)
        has_compute_queue = queue_family.queueFlags & VK_QUEUE_COMPUTE_BIT;
    }

    // Either no graphics or no compute queue family
    if (!has_graphics_queue || !has_compute_queue) {
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
    this->vk_physical_device_ = *devices_set.begin();
}

void Context::InitializeVkLogicalDevice() {
  // TODO: Implement logical device initialization
}

void Context::InitializeVkPipeline() {
  // TODO: Implement pipeline initialization
}

void Context::InitializeVkSwapChain() {
  // TODO: Implement swap chain initialization
}
