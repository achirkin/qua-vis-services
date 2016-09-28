#ifndef QUAVIS_SHADER_H
#define QUAVIS_SHADER_H

namespace quavis {
  /**
  * A wrapper around the vulkan shader modules.
  */
  class Shader {
  public:
    /**
    * Creates a new shader using the given SPIR-V code.
    */
    Shader(LogicalDevice device, const char* shader_code, uint32_t len);

    /**
    * The handler to object to be used by Vulkan methods.
    */
    VkShaderModule vk_handle;

  private:
    LogicalDevice device_;
  };
}

#endif
