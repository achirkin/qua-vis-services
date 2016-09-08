#ifndef QUAVIS_CONTEXT_H
#define QUAVIS_CONTEXT_H

#include "quavis/version.h"
#include "quavis/shaders.h"
#include "quavis/debug.h"

#include <vulkan/vulkan.h>
#include <memory>
#include <vector>
#include <set>

namespace quavis {
  /**
  * The Context class initializes and prepares the vulkan instance for fast
  * computations on the graphics card.
  */
  class Context {
  public:
    /**
    * Creates a new instance of the Context class. During its initialization,
    * the vulkan devices and pipelines are prepared for rendering / computation.
    */
    Context();

    /**
    * Destroy the object. All vulkan objects are cleanly removed here.
    */
    ~Context();

  private:
    void InitializeVkInstance();
    void InitializeVkPhysicalDevice();
    void InitializeVkLogicalDevice();
    void InitializeVkShaderModules();
    void InitializeVkRenderPass();
    void InitializeVkGraphicsPipeline();
    void InitializeVkGraphicsPipelineLayout();
    void InitializeVkMemory();
    void InitializeVkCommandPool();

    // instance data
    VkInstance vk_instance_;
    VkPhysicalDevice vk_physical_device_;
    VkDevice vk_logical_device_;
    uint32_t queue_family_index_;

    // queues
    VkQueue vk_queue_graphics_;
    VkQueue vk_queue_compute_;
    VkQueue vk_queue_transfer_;

    // shaders
    VkShaderModule vk_vertex_shader_;
    VkShaderModule vk_fragment_shader_;

    // pipeline
    VkRenderPass vk_render_pass_;
    VkPipelineLayout vk_pipeline_layout_;
    VkPipeline vk_pipeline_;

    // images
    VkImageView vk_color_imageview_;
    VkImageView vk_stencil_imageview_;
    VkImage vk_color_image_;
    VkImage vk_stencil_image_;
    VkDeviceMemory vk_color_image_memory_;
    VkDeviceMemory vk_stencil_image_memory_;


    // meta data for initialization
    const std::vector<const char*> vk_logical_device_extension_names_ = {};

    // rendering attributes
    const uint32_t render_width_ = 1000.0f;
    const uint32_t render_height_ = 1000.0f;
    const VkFormat color_format_ = VK_FORMAT_B8G8R8A8_UNORM;
    const VkFormat stencil_format_ = VK_FORMAT_D32_SFLOAT_S8_UINT;
  };
}

#endif
