#include "quavis/vk/instance.h"

namespace quavis {

Instance::Instance() {
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

  VkInstanceCreateInfo vkInstanceCreateInfo = VkInstanceCreateInfo {
    VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, // type (see documentation)
    nullptr, // next structure (see documentation)
    0, // flags, reserver by vulkan api for future api versions
    &vkApplicationInfo, // application info (see aboive)
    (uint32_t)this->layers_.size(), // number of layers (atm no debug/validation layers used)
    this->layers_.data(), // layer names
    (uint32_t)this->extensions_.size(), // number of extensions
    this->extensions_.data() // extension names
  };

  // create the instance object
  debug::handleVkResult(
    vkCreateInstance(
      &vkInstanceCreateInfo, // creation info (see above)
      nullptr, // allocation handler (gives specific info on memory locations)
      &this->vk_handle // pointer where to store the instance
    )
  );
}

Instance::~Instance {
  vkDestroyInstance(this->vk_handle, nullptr);
}

}
