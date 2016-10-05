#ifndef QUAVIS_QUAVIS_H
#define QUAVIS_QUAVIS_H

#include "quavis/version.h"
#include "quavis/shaders.h"
#include "quavis/vk/debug.h"
#include "quavis/vk/geometry/geometry.h"
#include "quavis/vk/geometry/vertex.h"
#include "quavis/vk/geometry/geojson.hpp"

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
  struct UniformBufferObject {
    vec3 observation_point;
    float r_max;
    float alpha_max;
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
    void CreateGraphicsDescriptorSet(VkDescriptorSetLayout layouts[], VkDescriptorSet* descriptor_set);
    void UpdateGraphicsDescriptorSet(uint32_t size, VkBuffer buffer, VkDescriptorSet* descriptor_set);
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
    void RetrieveRenderImage(uint32_t i);
    void RetrieveDepthImage();
    void RetrieveComputeImage();
    void* RetrieveResult();
    void ResetResult();

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
    VkShaderModule vk_geoemtry_shader_;
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
    VkBuffer vk_compute_staging_buffer_;
    VkBuffer vk_compute_buffer_;
    VkDeviceMemory vk_vertex_staging_buffer_memory_;
    VkDeviceMemory vk_vertex_buffer_memory_;
    VkDeviceMemory vk_index_staging_buffer_memory_;
    VkDeviceMemory vk_index_buffer_memory_;
    VkDeviceMemory vk_uniform_staging_buffer_memory_;
    VkDeviceMemory vk_uniform_buffer_memory_;
    VkDeviceMemory vk_compute_staging_buffer_memory_;
    VkDeviceMemory vk_compute_buffer_memory_;

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
    };

    // meta data for initialization
    const std::vector<const char*> vk_logical_device_extension_names_ = {
    };

    const std::vector<const char*> vk_validation_layers_ = {
    };


    // rendering attributes
    const uint32_t render_width_ = 128;
    const uint32_t render_height_ = 64;
    const size_t workgroups[3] = {16, 16, 1};
    const size_t num_observation_points_x = 500;
    const VkFormat color_format_ = VK_FORMAT_R32_SFLOAT;
    const VkFormat depth_stencil_format_ = VK_FORMAT_D32_SFLOAT;

    const uint32_t compute_size_ = sizeof(unsigned int);
    unsigned int compute_default_value_ = 0;

    std::vector<Vertex> vertices_ = {};
    std::vector<uint32_t> indices_ = {};
    UniformBufferObject uniform_ = {
      vec3 {0, 0, 0},
      100000,
      .5
    };
  };
}

#endif
