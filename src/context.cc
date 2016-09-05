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
  // Implement context destructor
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

}

void Context::InitializeVkPhysicalDevice() {
  // TODO: Implement physical device initialization
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
