#ifndef QUAVIS_CONTEXT_H
#define QUAVIS_CONTEXT_H

#include "quavis/version.h"
#include "quavis/shaders.h"
#include "quavis/debug.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <vulkan/vulkan.h>
#include <stdio.h>
#include <string.h>
#include <memory>
#include <vector>
#include <set>
#include <array>

namespace quavis {
  typedef struct vec2 {
    float x;
    float y;
  } vec2;

  typedef struct vec3 {
    float x;
    float y;
    float z;
  } vec3;

  struct Vertex {
      vec2 pos;
      vec3 color;

      static VkVertexInputBindingDescription getBindingDescription() {
          VkVertexInputBindingDescription bindingDescription = {};
          bindingDescription.binding = 0;
          bindingDescription.stride = sizeof(Vertex);
          bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
          return bindingDescription;
      }

      static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
          std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);
          attributeDescriptions[0].binding = 0;
          attributeDescriptions[0].location = 0;
          attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
          attributeDescriptions[0].offset = offsetof(Vertex, pos);

          attributeDescriptions[1].binding = 0;
          attributeDescriptions[1].location = 1;
          attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
          attributeDescriptions[1].offset = offsetof(Vertex, color);

          return attributeDescriptions;
      }
  };

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
    void InitializeVkCommandBuffers();
    void VkDraw();

    void CreateVertexBuffer();
    void CreateImage(VkFormat format, VkImageLayout layout, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags memoryflags, VkImage* image, VkDeviceMemory* image_memory);
    void CreateImageView(VkImage image, VkFormat format, VkImageAspectFlagBits flags, VkImageView* imageview);
    void CreateFrameBuffer();
    void CreateCommandPool();
    void CreateCommandBuffer();

    void TransformImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);
    void CopyImage(VkImage srcImage, VkImage dstImage, uint32_t width, uint32_t height);

    VkCommandBuffer BeginSingleTimeBuffer();
    void EndSingleTimeBuffer(VkCommandBuffer commandBuffer);

    void SubmitVertexData();
    void RetrieveImage();

    // instance data
    VkInstance vk_instance_;
    VkPhysicalDevice vk_physical_device_;
    VkDevice vk_logical_device_;
    uint32_t queue_family_index_;

    // command pool
    VkCommandPool vk_command_pool_;

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

    // vertex data
    VkBuffer vk_vertex_buffer_;
    VkDeviceMemory vk_vertex_buffer_memory_;

    // images
    VkImageView vk_color_imageview_;
    VkImage vk_color_image_;
    VkImage vk_host_visible_image_;
    VkDeviceMemory vk_color_image_memory_;
    VkDeviceMemory vk_host_visible_image_memory_;

    // framebuffers
    VkFramebuffer vk_graphics_framebuffer_;

    // command buffers
    VkCommandBuffer vk_graphics_commandbuffer_;

    // semaphores
    VkSemaphore vk_render_semaphore_;
    VkSemaphore vk_render_finished_semaphore_;

    // meta data for initialization
    const std::vector<const char*> vk_instance_extension_names_ = {
      "VK_EXT_debug_report"
    };

    // meta data for initialization
    const std::vector<const char*> vk_logical_device_extension_names_ = {
    };

    const std::vector<const char*> vk_validation_layers_ = {
      "VK_LAYER_LUNARG_standard_validation"
    };


    // rendering attributes
    const uint32_t render_width_ = 500;
    const uint32_t render_height_ = 500;
    const VkFormat color_format_ = VK_FORMAT_R8G8B8A8_UNORM;
    const VkFormat stencil_format_ = VK_FORMAT_D32_SFLOAT_S8_UINT;

    const std::vector<Vertex> vertices_ = {
        {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
    };
  };
}

#endif
