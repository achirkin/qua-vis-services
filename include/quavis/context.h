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
    void InitializeVkShaderModules();
    void InitializeVkGraphicsPipeline();

    VkInstance vk_instance_;
    VkPhysicalDevice vk_physical_device_;
    VkDevice vk_logical_device_;

    // queues
    VkQueue vk_queue_graphics_;
    VkQueue vk_queue_compute_;
    VkQueue vk_queue_transfer_;

    // shaders
    VkShaderModule vk_vertex_shader_;
    VkShaderModule vk_fragment_shader_;

    // meta data for initialization
    const std::vector<const char*> vk_logical_device_extension_names_ = {};

    // rendering attributes
    const uint32_t render_width_ = 1000.0f;
    const uint32_t render_height_ = 1000.0f;

    // TODO: Find better way to encode shader code in library
    const std::string vertex_shader_code_ = "#version 450"
    	"#extension GL_ARB_separate_shader_objects : enable"
    	""
    	"out gl_PerVertex {"
    	"    vec4 gl_Position;"
    	"};"
    	""
    	"layout(location = 0) out vec3 fragColor;"
    	""
    	"vec2 positions[3] = vec2[]("
    	"    vec2(0.0, -0.5),"
    	"    vec2(0.5, 0.5),"
    	"    vec2(-0.5, 0.5)"
    	");"
    	""
    	"vec3 colors[3] = vec3[]("
    	"    vec3(1.0, 0.0, 0.0),"
    	"    vec3(0.0, 1.0, 0.0),"
    	"    vec3(0.0, 0.0, 1.0)"
    	");"
    	""
    	"void main() {"
    	"    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);"
    	"    fragColor = colors[gl_VertexIndex];"
    	"}";

    const std::string fragment_shader_code_ = "#version 450"
    	"#extension GL_ARB_separate_shader_objects : enable"
    	""
    	"layout(location = 0) in vec3 fragColor;"
    	"layout(location = 0) out vec4 outColor;"
    	""
    	"void main() {"
    	"	   outColor = vec4(fragColor, 1.0);"
    	"}";
  };
}

#endif
