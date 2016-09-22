#ifndef QUAVIS_CONTEXT_H
#define QUAVIS_CONTEXT_H

#include "quavis/version.h"
#include "quavis/shaders.h"
#include "quavis/debug.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tinyobjloader.h"

#include <vulkan/vulkan.h>
#include <stdio.h>
#include <string.h>
#include <memory>
#include <vector>
#include <set>
#include <array>

namespace quavis {
  typedef struct vec3 {
    float x;
    float y;
    float z;
  } vec3;

  typedef struct mat4 {
    float data[16];
  } mat4;

  struct UniformBufferObject {
    vec3 observation_point;
    float r_max;
  };

  struct Vertex {
      vec3 pos;
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
          attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
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
    void InitializeVkDescriptorPool();
    void InitializeVkDescriptorSetLayout();
    void InitializeVkGraphicsPipelineLayout();
    void InitializeVkComputePipelineLayout();
    void InitializeVkGraphicsPipeline();
    void InitializeVkComputePipeline();
    void InitializeVkMemory();
    void InitializeVkGraphicsCommandBuffers();
    void InitializeVkComputeCommandBuffers();
    void InitializeVkImageLayouts();
    void VkDraw();
    void VkCompute();

    void CreateBuffer(VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryflags, uint32_t size, VkBuffer* buffer, VkDeviceMemory* buffer_memory);
    void CreateImage(VkFormat format, VkImageLayout layout, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags memoryflags, VkImage* image, VkDeviceMemory* image_memory);
    void CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags flags, VkImageView* imageview);
    void CreateAndUpdateDescriptorSet(VkDescriptorSetLayout layouts[], uint32_t size, VkBuffer buffer, VkDescriptorSet* descriptor_set);
    void CreateComputeDescriptorSets();
    void UpdateComputeDescriptorSets();
    void CreateFrameBuffer();
    void CreateCommandPool(VkCommandPool* pool);
    void CreateCommandBuffer(VkCommandPool pool, VkCommandBuffer* buffer);

    void TransformImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageAspectFlags flags);
    void CopyImage(VkImage srcImage, VkImage dstImage, uint32_t width, uint32_t height, VkImageAspectFlags aspectFlags);

    VkCommandBuffer BeginSingleTimeBuffer();
    void EndSingleTimeBuffer(VkCommandBuffer commandBuffer);

    void SubmitVertexData();
    void SubmitIndexData();
    void SubmitUniformData();
    void RetrieveRenderImage();
    void RetrieveDepthImage();
    void RetrieveComputeImage();

    // instance data
    VkInstance vk_instance_;
    VkPhysicalDevice vk_physical_device_;
    VkDevice vk_logical_device_;
    uint32_t queue_family_index_;

    // FENCES
    VkFence vk_compute_fence_;

    // command pool
    VkCommandPool vk_graphics_command_pool_;
    VkCommandPool vk_compute_command_pool_;

    // queues
    VkQueue vk_queue_graphics_;
    VkQueue vk_queue_compute_;
    VkQueue vk_queue_transfer_;

    // shaders
    VkShaderModule vk_vertex_shader_;
    VkShaderModule vk_tessellation_control_shader_;
    VkShaderModule vk_tessellation_evaluation_shader_;
    VkShaderModule vk_fragment_shader_;
    VkShaderModule vk_compute_shader_;

    // pipeline
    VkRenderPass vk_render_pass_;
    VkPipelineLayout vk_graphics_pipeline_layout_;
    VkPipelineLayout vk_compute_pipeline_layout_;
    VkPipeline vk_graphics_pipeline_;
    VkPipeline vk_compute_pipeline_;

    // descriptors
    VkDescriptorPool vk_descriptor_pool_;
    VkDescriptorSetLayout vk_graphics_descriptor_set_layout_;
    VkDescriptorSetLayout vk_compute_descriptor_set_layout_;
    VkDescriptorSet vk_graphics_descriptor_set_;
    VkDescriptorSet vk_compute_descriptor_set_;

    // vertex data
    VkBuffer vk_vertex_staging_buffer_;
    VkBuffer vk_vertex_buffer_;
    VkBuffer vk_index_staging_buffer_;
    VkBuffer vk_index_buffer_;
    VkBuffer vk_uniform_staging_buffer_;
    VkBuffer vk_uniform_buffer_;
    VkDeviceMemory vk_vertex_staging_buffer_memory_;
    VkDeviceMemory vk_vertex_buffer_memory_;
    VkDeviceMemory vk_index_staging_buffer_memory_;
    VkDeviceMemory vk_index_buffer_memory_;
    VkDeviceMemory vk_uniform_staging_buffer_memory_;
    VkDeviceMemory vk_uniform_buffer_memory_;

    // images
    VkImageView vk_color_imageview_;
    VkImageView vk_depth_stencil_imageview_;
    VkImageView vk_compute_imageview_;
    VkImage vk_color_image_;
    VkImage vk_depth_stencil_image_;
    VkImage vk_compute_image_;
    VkImage vk_color_staging_image_;
    VkImage vk_depth_stencil_staging_image_;
    VkDeviceMemory vk_color_image_memory_;
    VkDeviceMemory vk_depth_stencil_image_memory_;
    VkDeviceMemory vk_compute_image_memory_;
    VkDeviceMemory vk_color_staging_image_memory_;
    VkDeviceMemory vk_depth_stencil_staging_image_memory_;

    // sampler
    VkSampler vk_sampler_;

    // framebuffers
    VkFramebuffer vk_graphics_framebuffer_;

    // command buffers
    VkCommandBuffer vk_graphics_commandbuffer_;
    VkCommandBuffer vk_compute_commandbuffer_;

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
    const uint32_t render_width_ = 2048;
    const uint32_t render_height_ = 1024;
    const VkFormat color_format_ = VK_FORMAT_R8G8B8A8_UNORM;
    const VkFormat depth_stencil_format_ = VK_FORMAT_D32_SFLOAT;

    std::vector<Vertex> vertices_ = {
      {{5,5,4},{0,0,0}},
      {{5,5,5},{0,0,0}},
      {{5,0,5},{0,0,0}},
      {{-10,-1,0},{0,0,0}},
      {{-10,-1,1},{0,0,0}},
      {{-10,0,2},{0,0,0}},
    };

    std::vector<uint32_t> indices_ = {
      0,1,2,3,4,5
    };

    const UniformBufferObject uniform_ = {
      vec3 {0, 0, 1},
      100.0
    };
  };
}

#endif
