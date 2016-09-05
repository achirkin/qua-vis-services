#ifndef QUAVIS_CONTEXT_H
#define QUAVIS_CONTEXT_H

#include "quavis/vk/physicaldevice.h"
#include "quavis/vk/logicaldevice.h"
#include "quavis/vk/pipeline.h"
#include "quavis/vk/swapchain.h"

#include <vulkan/vulkan.h>
#include <memory>

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

    std::unique_ptr<vk::PhysicalDevice> physical_device_;
    std::unique_ptr<vk::LogicalDevice> logical_device_;
    std::unique_ptr<vk::Pipeline> pipeline_;
    std::unique_ptr<vk::Swapchain> swapchain_;
  };
}

#endif
