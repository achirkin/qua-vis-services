#ifndef QUAVIS_CONTEXT_H
#define QUAVIS_CONTEXT_H

#include "quavis/vk/physicaldevice.h"
#include "quavis/vk/logicaldevice.h"
#include "quavis/vk/pipeline.h"
#include "quavis/vk/swapchain.h"

#include <vulkan/vulkan.h>
#include <vector>
#include <memory>

namespace quavis {
  class Context {
  public:
    Context() : physical_devices_(), logical_devices_() {

    }

  private:
    void initializeVkInstance();
    void initializeVkPhysicalDevice();
    void initializeVkLogicalDevice();
    void initializeVkSwapChain();
    void initializeVkPipeline();

    std::vector<PhysicalDevice> physical_devices_;
    std::vector<LogicalDevice> logical_devices_;
    std::unique_ptr<Pipeline> pipeline_;
    std::unique_ptr<Swapchain> swapchain_;
  };
}

#endif
