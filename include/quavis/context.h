#ifndef QUAVIS_CONTEXT_H
#define QUAVIS_CONTEXT_H

#include "quavis/version.h"
#include "quavis/vk/result.h"

#include <vulkan/vulkan.h>
#include <memory>
#include <vector>
#include <set>

namespace quavis {
  class Context {
  public:
    Context();
    ~Context();

  private:
    void InitializeVkInstance();
    void InitializeVkPhysicalDevice();
    void InitializeVkLogicalDevice();
    void InitializeVkPipeline();
    void InitializeVkSwapChain();

    VkInstance vk_instance_;
    VkPhysicalDevice vk_physical_device_;
  };
}

#endif
