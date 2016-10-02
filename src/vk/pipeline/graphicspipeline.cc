#include "quavis/vk/pipeline/graphicspipeline.h"

namespace quavis {
  GraphicsPipeline::GraphicsPipeline(std::shared_ptr<LogicalDevice> device,
    std::vector<std::shared_ptr<DescriptorSet>> descriptor_sets,
    std::vector<std::shared_ptr<Shader>> shaders,
    std::shared_ptr<Buffer> vertex_buffer,
    std::shared_ptr<Buffer> index_buffer,
    std::shared_ptr<Image> color_image,
    std::shared_ptr<Image> depth_image) : Pipeline(device, descriptor_sets, shaders) {
      this->vertex_buffer_ = vertex_buffer;
      this->index_buffer_ = index_buffer;
      this->color_image_ = color_image;
      this->depth_image_ = depth_image;

      this->vk_render_pass_ = this->InitializeRenderPass();
      this->vk_framebuffer_ = this->InitializeFramebuffer();
      this->vk_handle = this->InitializePipeline();
    }

  GraphicsPipeline::~GraphicsPipeline() {
    vkDestroyRenderPass(this->logical_device_->vk_handle, this->vk_render_pass_, nullptr);
    vkDestroyFramebuffer(this->logical_device_->vk_handle, this->vk_framebuffer_, nullptr);
    vkDestroyPipeline(this->logical_device_->vk_handle, this->vk_handle, nullptr);

  }

  std::vector<VkPipelineShaderStageCreateInfo> GraphicsPipeline::InitializeShaderInfos() {
    std::vector<VkPipelineShaderStageCreateInfo> shader_infos;
    for(uint32_t i = 0; i < this->shaders_.size(); i++) {
      VkPipelineShaderStageCreateInfo stage_info = {
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, // sType (see documentation)
        nullptr, // next (see documentation, must be null)
        0, // flags (see documentation, must be 0)
        this->shaders_[i]->vk_shader_stage, // stage flag
        this->shaders_[i]->vk_handle, // shader module
        "main", // the pipeline's name
        nullptr // VkSpecializationInfo (see documentation)
      };
      shader_infos.push_back(stage_info);
    }

    return shader_infos;
  }

  std::vector<VkAttachmentDescription> GraphicsPipeline::InitializeAttachments() {
    // create attachment descriptions for color / stencil
    VkAttachmentDescription color_attachment_description = {
      0, // flags (see documentation, 1 option)
      this->color_image_->format, // color format
      VK_SAMPLE_COUNT_1_BIT, // num samples per fragment
      VK_ATTACHMENT_LOAD_OP_CLEAR, // operation when loading
      VK_ATTACHMENT_STORE_OP_STORE, // operation when storing
      VK_ATTACHMENT_LOAD_OP_DONT_CARE, // stencil operation when loading
      VK_ATTACHMENT_STORE_OP_DONT_CARE, // stencil operation when storing
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, // initial layout
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL // final layout
    };

    VkAttachmentDescription depth_attachment_description = {
      0, // flags (see documentation, 1 option)
      this->depth_image_->format, // color format
      VK_SAMPLE_COUNT_1_BIT, // num samples per fragment
      VK_ATTACHMENT_LOAD_OP_CLEAR, // operation when loading
      VK_ATTACHMENT_STORE_OP_STORE, // operation when storing
      VK_ATTACHMENT_LOAD_OP_DONT_CARE, // stencil operation when loading
      VK_ATTACHMENT_STORE_OP_DONT_CARE, // stencil operation when storing
      VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, // initial layout
      VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL // final layout
    };

    std::vector<VkAttachmentDescription> attachment_descriptions = {
      color_attachment_description,
      depth_attachment_description
    };

    return attachment_descriptions;
  }

  VkRenderPass GraphicsPipeline::InitializeRenderPass() {
    std::vector<VkAttachmentDescription> attachments = this->InitializeAttachments();

    // create attachment refernces for color / stencil
    VkAttachmentReference color_attachment_reference = {
      0, // index
      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL // layout
    };

    VkAttachmentReference depth_stencil_attachment_reference = {
      1, // index
      VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL // layout
    };

    // create subpass for attachments
    VkSubpassDescription subpass_description = {
      0, // flags (see documentation, must be 0)
      VK_PIPELINE_BIND_POINT_GRAPHICS, // bind point (graphics / compute)
      0, // input attachment count(0 for now) // TODO: Add correct vertex input
      nullptr, // input attachments
      1, // color attachment count
      &color_attachment_reference, // color attachment references
      nullptr, // resolve attachment references
      &depth_stencil_attachment_reference, // stencil attachment
      0, // preserved attachment count
      nullptr // preserved attachments
    };

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependency.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    // create render pass
    VkRenderPassCreateInfo render_pass_info = {
      VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, // sType
      nullptr, // next (see documentation, must be null)
      0, // flags
      (uint32_t)attachments.size(), // attachment count
      attachments.data(), // attachment descriptions
      1, // subpass count
      &subpass_description, // subpass
      1, // dependency count between subpasses
      &dependency // dependencies
    };

    VkRenderPass render_pass;

    vkCreateRenderPass(
      this->logical_device_->vk_handle,
      &render_pass_info,
      nullptr,
      &render_pass
    );

    return render_pass;
  }

  VkPipeline GraphicsPipeline::InitializePipeline() {
    std::vector<VkPipelineShaderStageCreateInfo> shader_infos = this->InitializeShaderInfos();

    // Get vertex binding and attributes
    VkVertexInputBindingDescription vertex_binding = Vertex::getBindingDescription();
    std::vector<VkVertexInputAttributeDescription> vertex_attributes = Vertex::getAttributeDescriptions();

    // Define input loading
    VkPipelineVertexInputStateCreateInfo vertex_input_info {
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO, // sType
      nullptr, // next (see documentation, must be null)
      0, // flags (see documentation, must be 0)
      1, // binding description count (spacing of data etc.)
      &vertex_binding, // binding descriptions, here we don't load vertices atm
      (uint32_t)vertex_attributes.size(), // attribute description count (types passed to vertex shader etc.)
      vertex_attributes.data() // attribute descriptions
    };

    VkPipelineInputAssemblyStateCreateInfo input_assembly_info {
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, // sType
      nullptr, // pNext (see documentation, must be null)
      0, // flags (see documentation, must be 0)
      VK_PRIMITIVE_TOPOLOGY_PATCH_LIST, // topology of vertices
      VK_FALSE // whether there should be a special vertex index to reassemble
    };

    // Define viewport
    VkViewport viewport = {
      0.0f, // upper left corner x
      0.0f, // upper left corner y
      (float)this->color_image_->width, // width
      (float)this->color_image_->height, // height
      0.0f, // min depth
      1.0f // max depth
    };

    VkRect2D scissor = {
      {0, 0}, // offset (casted to VkOffset2D)
      {this->color_image_->width, this->color_image_->height} // extent (casted to VkExtent2D)
    };

    VkPipelineViewportStateCreateInfo viewport_info = {
      VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO, // sType
      nullptr, // next (see documentation, must be null)
      0, // flags (see documentation, must be 0)
      1, // viewport count
      &viewport, // viewport
      1, // scissor count
      &scissor, // scissor
    };

    // Define rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizer_info = {
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO, // sType
      nullptr, // next (see documentation, must be null)
      0, // flags (see documentation, must be 0)
      VK_FALSE, // depth clamping
      VK_FALSE, // discard primitives before rendering?
      VK_POLYGON_MODE_FILL, // fill polygons (alternatively: draw only edges / vertices)
      VK_CULL_MODE_NONE, // discard one of the two faces of a polygon
      VK_FRONT_FACE_COUNTER_CLOCKWISE, // counter clockwise = front
      VK_FALSE, // depth bias // TODO: Find out whether we need depth bias
      0.0f, // depth bias constant
      0.0f, // depth bias clamp
      0.0f, // depth bias slope
      1.0f // line width // TODO: Change line width to 0
    };

    // Define Sampler
    VkPipelineMultisampleStateCreateInfo multisampling_info {
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO, // sType
      nullptr, // next (see documentation, must be null)
      0, // flags (see documentation, must be 0)
      VK_SAMPLE_COUNT_1_BIT, // rasterize per pixel (no anti-aliasing)
      VK_FALSE, // shade per fragment
      1.0f, // minimum sample shading
      nullptr, // sampling mask
      VK_FALSE, // alpha-to-converge
      VK_FALSE // alpha-to-one
    };

    // Define Blending
    VkPipelineColorBlendAttachmentState color_blend_attachment_state {
      VK_FALSE, // don't use the blending // TODO: If we want transparency, add blending
      VK_BLEND_FACTOR_ONE, // blending factor source (color)
      VK_BLEND_FACTOR_ZERO, // blending factor target (color)
      VK_BLEND_OP_ADD, // blending operation (color)
      VK_BLEND_FACTOR_ONE, // blending factor source (alpha)
      VK_BLEND_FACTOR_ZERO, // blending factor target (alpha)
      VK_BLEND_OP_ADD, // blending operation (alpha)
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT // color mask
    };

    VkPipelineColorBlendStateCreateInfo color_blend_info {
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO, // sType
      nullptr, // next (see documentation, must be null)
      0, // flags (see documentation, must be 0)
      VK_FALSE, // whether to combine framebuffers logically after first blending
      VK_LOGIC_OP_COPY, // logical operation
      1, // attachment count
      &color_blend_attachment_state, // attachment state
      {0.0f, 0.0f, 0.0f, 0.0f} // logical rgba factors
    };

    // TODO: Define VkPipelineDepthStencilStateCreateInfo correctly
    // Define DepthStencil testing
    VkPipelineDepthStencilStateCreateInfo depth_stencil_info {
      VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO, // sType
      nullptr, // next (see documentation, must be null)
      0, // flags (see documentation, must be 0)
      VK_TRUE, // test depth
      VK_TRUE, // write depth
      VK_COMPARE_OP_LESS, // comparison operation
      VK_FALSE, // depth bound test
      VK_FALSE, // stencil test
      {}, // front stencil op state
      {}, // back stencil op state
      0.0f, // min depth
      1.0f // max depth
    };

    VkPipelineTessellationStateCreateInfo tessellation_info = {
      VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO, // sType
      nullptr, // next (see documentation, must be null)
      0, // flags (see documentation, must be 0)
      3 // number of vertices per patch
    };

    // Define pipeline info
    VkGraphicsPipelineCreateInfo pipeline_info = {
      VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO, // sType
      nullptr, // next (see documentation, must be null)
      0, // pipeline create flags (have no child pipelines, so don't care)
      (uint32_t)shader_infos.size(), // number of stages (we have 2 shaders for now)
      shader_infos.data(), // shader stage create infos
      &vertex_input_info, // vertex input info
      &input_assembly_info, // inpt assembly info
      &tessellation_info, // tesselation info
      &viewport_info, // viewport info
      &rasterizer_info, // rasterization info
      &multisampling_info, // multisampling info
      &depth_stencil_info, // depth stencil info
      &color_blend_info, // blending info
      nullptr, // dynamic states info (e.g. window size changes or so)
      this->vk_layout, // pipeline layout
      this->vk_render_pass_, // render pass
      0, // subpass index for this pipeline (we only have 1)
      VK_NULL_HANDLE, // parent pipeline
      -1 // parent pipeline index
    };

    VkPipeline pipeline;

    vkCreateGraphicsPipelines(
      this->logical_device_->vk_handle, // logical device
      VK_NULL_HANDLE, // pipeline cache // TODO: Add pipeline cache (?)
      1, // pipeline count
      &pipeline_info, // pipeline infos
      nullptr, // allocation callback
      &pipeline // allocated memory for the pipeline
    );

    return pipeline;
  }

  VkFramebuffer GraphicsPipeline::InitializeFramebuffer() {
    VkFramebuffer framebuffer;

    // Create Framebuffers for color & stencil (color & depth)
    std::vector<VkImageView> attachments = {
      this->color_image_->vk_view,
      this->depth_image_->vk_view
    };

    VkFramebufferCreateInfo framebuffer_info = {
      VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, // sType
      nullptr,// pNext (see documentation, must be null)
      0, // flags (see documentation, must be 0)
      this->vk_render_pass_, // render pass
      (uint32_t)attachments.size(), // attachment count
      attachments.data(), // attachments
      this->color_image_->width, // width
      this->color_image_->height, // height
      1 // layer count
    };

    vkCreateFramebuffer(
      this->logical_device_->vk_handle, // the logical device
      &framebuffer_info, // info
      nullptr, // allocation callback
      &framebuffer // the allocated memory
    );

    return framebuffer;
  }

  VkCommandBuffer GraphicsPipeline::CreateCommandBuffer() {
    VkCommandBuffer command_buffer = this->logical_device_->BeginCommandBuffer(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

    VkClearValue clear_values[] = {
      {0.0f, 0.0f, 0.0f, 1.0f},
      {1.0f, 0.0f}
    };

    VkRenderPassBeginInfo render_pass_info = {
      VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO, // sType
      nullptr, // pNext (see documentation, must be null)
      this->vk_render_pass_, // render pass
      this->vk_framebuffer_, // framebuffer
      {{0,0}, {this->color_image_->width, this->color_image_->height}}, // render area (VkRect2D)
      2, // number of clear values
      clear_values // clear values
    };

    vkCmdBeginRenderPass(
      command_buffer, // command buffer
      &render_pass_info, // render pass info
      VK_SUBPASS_CONTENTS_INLINE // store contents in primary command buffer
    );

    // bind graphics pipeline
    vkCmdBindPipeline(
      command_buffer, // command buffer
      VK_PIPELINE_BIND_POINT_GRAPHICS, // pipeline type
      this->vk_handle // graphics pipeline
    );

    // vertex data
    VkBuffer vertexBuffers[] = {this->vertex_buffer_->vk_handle};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(command_buffer,
      0, // vertex buffer binding index
      1, // number of bindings
      vertexBuffers, // vertex buffers
      offsets // offsets
    );

    std::vector<VkDescriptorSet> descriptor_sets;
    for (uint32_t i = 0; i < this->descriptor_sets_.size(); i++) {
      descriptor_sets.push_back(this->descriptor_sets_[i]->vk_handle);
    }

    // uniform buffer object
    vkCmdBindDescriptorSets(
      command_buffer,
      VK_PIPELINE_BIND_POINT_GRAPHICS,
      this->vk_layout,
      0,
      (uint32_t)descriptor_sets.size(),
      descriptor_sets.data(),
      0,
      nullptr
    );

    vkCmdBindIndexBuffer(command_buffer, this->index_buffer_->vk_handle, 0, VK_INDEX_TYPE_UINT32);

    // draw
    vkCmdDrawIndexed(
      command_buffer, // command buffer
      this->index_buffer_->size / sizeof(uint32_t), // num indexes
      1, // num instances // TODO
      0, // first index
      0, // vertex index offset
      0 // first instance
    );

    this->logical_device_->EndCommandBuffer(command_buffer);

    return command_buffer;
  }
}
