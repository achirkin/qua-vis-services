#ifndef QUAVIS_SHADER_H
#define QUAVIS_SHADER_H

#include "quavis/vk/device/logicaldevice.h"
#include "quavis/vk/debug.h"

#include <vulkan/vulkan.h>

namespace quavis {
  /**
  * A wrapper around the vulkan shader modules.
  */
  class Shader {
  public:
    /**
    * Creates a new shader using the given SPIR-V code.
    */
    Shader(LogicalDevice* device, VkShaderStageFlags shader_stage, const char* shader_code, uint32_t size);

    /**
    * The handler to object to be used by Vulkan methods.
    */
    VkShaderModule vk_handle;

    /**
    * The shader's stage
    */
    VkShaderStageFlags vk_shader_stage;

  private:
    LogicalDevice* logical_device_;
  };
}

#endif
