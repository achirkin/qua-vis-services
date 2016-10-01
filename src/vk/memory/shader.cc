#include "quavis/vk/memory/shader.h"

namespace quavis {
  Shader::Shader(LogicalDevice* device, VkShaderStageFlagBits shader_stage, const char* shader_code, uint32_t size) {
    this->logical_device_ = device;
    this->vk_shader_stage = shader_stage;

    // create vertex shader
    VkShaderModuleCreateInfo vertex_shader_info = {
      VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, // type (see documentation)
      nullptr, // next (see documentation, must be null)
      0, // flags (see documentation, must be 0)
      size, // vertex shader size
      (uint32_t*)shader_code // vertex shader code
    };

    vkCreateShaderModule(
      this->logical_device_->vk_handle, // the logical device
      &vertex_shader_info, // shader meta data
      nullptr, // allocation callback (see documentation)
      &this->vk_handle // the allocated memory for the logical device
    );
  }

  Shader::~Shader() {
    vkDestroyShaderModule(this->logical_device_->vk_handle, this->vk_handle, nullptr);
  }
}
