#include "quavis/context.h"

using namespace quavis;

Context::Context() {
  this->InitializeVkInstance();
  this->InitializeVkPhysicalDevice();
  this->InitializeVkLogicalDevice();
  this->InitializeVkPipeline();
}

Context::~Context() {
  vkDestroyInstance(this->vk_instance_, nullptr);
  vkDestroyDevice(this->vk_logical_device_, nullptr);
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
  // TODO: Check devices for extension support
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

    /////////////////// BEGIN CHECK EXTENSION SUPPORT
    // get number of supported extension in device
    uint32_t extension_count;
    vkEnumerateDeviceExtensionProperties(
      *device_it, // the physical device
      nullptr, // the layer name
      &extension_count, // the allocated memory for the number of extensions
      nullptr // the allocated memory for the extension properties
    );

    // get device's supported extensions
    std::vector<VkExtensionProperties> available_extensions(extension_count);
    vkEnumerateDeviceExtensionProperties(
      *device_it, // the physical device
      nullptr, // the layer name
      &extension_count, // the allocated memory for the number of extensions
      available_extensions.data() // the allocated memory for the extension properties
    );

    std::set<std::string> required_extensions(this->vk_logical_device_extension_names_.begin(), vk_logical_device_extension_names_.end());
    for (VkExtensionProperties extension : available_extensions) {
        required_extensions.erase(extension.extensionName);
    }

    // check all requirements
    if (!required_extensions.empty()) {
      device_it = devices_set.erase(device_it);
      continue;
    }

    /////////////////// END REQUIREMENT CHECK
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
  // get graphics and compute queue family index for selected device

  uint32_t num_queue_families = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(
    this->vk_physical_device_, // the vk device
    &num_queue_families, // the allocated memory for the number of families
    nullptr // the allocated memory for the queue family properties
  );

  // get queues
  std::vector<VkQueueFamilyProperties> queue_families(num_queue_families);
  vkGetPhysicalDeviceQueueFamilyProperties(
    this->vk_physical_device_, // the vk device
    &num_queue_families, // the allocated memory for the number of families
    queue_families.data() // the allocated memory for the queue family properties
  );

  // get index of suitible queue family
  // TODO: Maybe separate graphics, compute and transfer queue families
  uint32_t queue_family_index = 0;
  for (VkQueueFamilyProperties queue_family : queue_families) {
    uint32_t requirements = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;
    if (queue_family.queueFlags & requirements)
      break;
    queue_family_index++;
  }

  // Create graphics queue metadata
  // TODO: Seperate queue construction if multiple families are required
  float queue_family_priorities[] = { 1.0f, 1.0f, 1.0f };
  VkDeviceQueueCreateInfo queue_create_info {
    VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, // sType (see documentation)
    nullptr, // next structure (see documentation)
    0, // queue flags MUST be 0 (see documentation)
    queue_family_index, // queue family
    3, // number of queues: Graphics, Compute, Transfer
    queue_family_priorities // queue priority
  };

  // Specify device features
  // TODO: Specify device features
  VkPhysicalDeviceFeatures device_features = {};

  // Create lgocial device metadata
  VkDeviceCreateInfo device_create_info = {
    VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, // sType (see documentation)
    nullptr, // next (see documentation)
    0, // flags (see documentation)
    1, // number of queue create info objects
    &queue_create_info, // queue meta data
    0, // deprecated & ignored
    nullptr, // depcrecated & ignored
    (uint32_t)this->vk_logical_device_extension_names_.size(), // enabled extensions
    this->vk_logical_device_extension_names_.data(), // extension names
    &device_features // enabled device features
  };

  // create logical device
  vk::handleVkResult(
    vkCreateDevice(
      this->vk_physical_device_, // the physical device
      &device_create_info, // logical device metadata
      nullptr, // allocation callback (see documentation)
      &this->vk_logical_device_ // the allocated memory for the logical device
    )
  );
  
  // get graphics queue
  vkGetDeviceQueue(
    this->vk_logical_device_, // the logical device
    queue_family_index, // the queue family from which we want the queue
    0, // the index of the queue we want < NUM_QUEUES_IN_FAMILY
    &this->vk_queue_graphics_ // the allocated memory for the queue
  );

  // get compute queue
  vkGetDeviceQueue(
    this->vk_logical_device_, // the logical device
    queue_family_index, // the queue family from which we want the queue
    1, // the index of the queue we want < NUM_QUEUES_IN_FAMILY
    &this->vk_queue_compute_ // the allocated memory for the queue
  );

  // get transfer queue
  vkGetDeviceQueue(
    this->vk_logical_device_, // the logical device
    queue_family_index, // the queue family from which we want the queue
    2, // the index of the queue we want < NUM_QUEUES_IN_FAMILY
    &this->vk_queue_transfer_ // the allocated memory for the queue
  );
}

void Context::InitializeVkPipeline() {
  // TODO: Implement pipeline initialization
}
