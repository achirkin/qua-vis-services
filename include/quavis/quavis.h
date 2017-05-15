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
#include <ctime>

// glm
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace quavis {
  struct UniformBufferObject {
    glm::mat4 projection;
    glm::mat4 view;
    glm::mat4 model;
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
    Context(std::string cp_shader_1,
    std::string cp_shader_2,
    bool debug,
    bool line,
    int timing,
    bool disable_geom,
    bool disable_tess,
    int render_width,
    int workgroups);

    std::vector<float> Parse(std::string path, std::vector<vec3> analysispoints, float alpha_min, float r_max);

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
    void CreateImage(VkFormat format, VkImageLayout layout, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags memoryflags, VkImage* image, VkDeviceMemory* image_memory, uint32_t layers, VkExtent3D extent);
    void CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags flags, VkImageView* imageview, uint32_t layer);
    void CreateGraphicsDescriptorSet(VkDescriptorSetLayout layouts[], VkDescriptorSet* descriptor_set);
    void UpdateGraphicsDescriptorSet(uint32_t size, VkBuffer buffer, VkDescriptorSet* descriptor_set);
    void CreateComputeDescriptorSets();
    void UpdateComputeDescriptorSets();
    void CreateFrameBuffer();
    void CreateCommandPool(VkCommandPool* pool);
    void CreateCommandBuffer(VkCommandPool pool, VkCommandBuffer* buffer);

    void TransformImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageAspectFlags flags, uint32_t layers);
    void CopyImage(VkImage srcImage, VkImage dstImage, uint32_t width, uint32_t height, VkImageAspectFlags aspectFlags, uint32_t layer, VkOffset3D dstOffset);

    VkCommandBuffer BeginSingleTimeBuffer();
    void EndSingleTimeBuffer(VkCommandBuffer commandBuffer);

    void SubmitVertexData();
    void SubmitIndexData();
    void SubmitUniformData();
    void RetrieveRenderImage(uint32_t i);
    void RetrieveDepthImage(uint32_t i);
    void RetrieveComputeImage();
    void* RetrieveResult();
    void ResetResult();

    char* cp_shader_1_src_;
    char* cp_shader_2_src_;
    size_t cp_shader_1_len_;
    size_t cp_shader_2_len_;


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
    VkShaderModule vk_compute_shader_2_;

    // pipeline
    VkRenderPass vk_render_pass_;
    VkPipelineLayout vk_graphics_pipeline_layout_;
    VkPipelineLayout vk_compute_pipeline_layout_;
    std::vector<VkPipeline> vk_graphics_pipelines_;
    VkPipeline vk_compute_pipeline_;
    VkPipeline vk_compute_pipeline_2_;

    // descriptors
    VkDescriptorPool vk_descriptor_pool_;
    VkDescriptorSetLayout vk_graphics_descriptor_set_layout_;
    VkDescriptorSetLayout vk_compute_descriptor_set_layout_;
    VkDescriptorSetLayout vk_compute_out_descriptor_set_layout_;
    VkDescriptorSet vk_graphics_descriptor_set_;
    VkDescriptorSet vk_compute_descriptor_set_;
    VkDescriptorSet vk_compute_out_descriptor_set_;

    // vertex data
    VkBuffer vk_vertex_staging_buffer_;
    VkBuffer vk_vertex_buffer_;
    VkBuffer vk_index_staging_buffer_;
    VkBuffer vk_index_buffer_;
    VkBuffer vk_uniform_staging_buffer_;
    VkBuffer vk_uniform_buffer_;
    VkBuffer vk_compute_staging_buffer_;
    VkBuffer vk_compute_buffer_;
    VkBuffer vk_compute_tmp_buffer_;
    VkDeviceMemory vk_vertex_staging_buffer_memory_;
    VkDeviceMemory vk_vertex_buffer_memory_;
    VkDeviceMemory vk_index_staging_buffer_memory_;
    VkDeviceMemory vk_index_buffer_memory_;
    VkDeviceMemory vk_uniform_staging_buffer_memory_;
    VkDeviceMemory vk_uniform_buffer_memory_;
    VkDeviceMemory vk_compute_staging_buffer_memory_;
    VkDeviceMemory vk_compute_buffer_memory_;
    VkDeviceMemory vk_compute_tmp_buffer_memory_;

    // images
    VkImageView vk_color_imageview_1_;
    VkImageView vk_color_imageview_2_;
    VkImageView vk_color_imageview_3_;
    VkImageView vk_color_imageview_4_;
    VkImageView vk_color_imageview_5_;
    VkImageView vk_color_imageview_6_;
    VkImageView vk_depth_stencil_imageview_1_;
    VkImageView vk_depth_stencil_imageview_2_;
    VkImageView vk_depth_stencil_imageview_3_;
    VkImageView vk_depth_stencil_imageview_4_;
    VkImageView vk_depth_stencil_imageview_5_;
    VkImageView vk_depth_stencil_imageview_6_;
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
    VkCommandBuffer vk_compute_commandbuffer_2_;

    // semaphores
    VkSemaphore vk_render_semaphore_;
    VkSemaphore vk_render_finished_semaphore_;

    // meta data for initialization
    const std::vector<const char*> vk_instance_extension_names_ = {
      //"VK_EXT_debug_report"
    };

    // meta data for initialization
    const std::vector<const char*> vk_logical_device_extension_names_ = {
    };

    const std::vector<const char*> vk_validation_layers_ = {
      /*"VK_LAYER_GOOGLE_threading",
			"VK_LAYER_LUNARG_parameter_validation",
			"VK_LAYER_LUNARG_object_tracker",
			"VK_LAYER_LUNARG_core_validation",
			"VK_LAYER_LUNARG_swapchain",
			"VK_LAYER_GOOGLE_unique_objects"*/
    };

    // rendering attributes
    uint32_t render_width_;
    uint32_t render_height_;
    size_t workgroups[3] = {1, 1, 1}; // set in constructor
    size_t workgroups2[3] = {1, 1, 1};
    const VkFormat color_format_ = VK_FORMAT_R32G32_SFLOAT;
    const VkFormat depth_stencil_format_ = VK_FORMAT_D32_SFLOAT;

    const uint32_t compute_size_ = sizeof(float);
    unsigned int compute_default_value_ = 0;

    std::vector<Vertex> vertices_ = {};
    std::vector<uint32_t> indices_ = {};
    UniformBufferObject uniform_ = {};

    // flags
    bool debug_mode_;
    bool line_mode_;
    int timing_mode_;
    bool disable_geom_;
    bool disable_tess_;

    // timings
    std::clock_t start_time_;
    double init_memory_time_;
    double init_image_layout_time_;
    double submission_time_;
    double graphics_time_ = 0;
    double compute_time_ = 0;
    double image_retrieval_time_ = 0;
    double image_storage_time_ = 0;
    double result_retrieval_time_ = 0;
  };
}

#endif
