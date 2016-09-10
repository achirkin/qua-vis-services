#include "quavis/context.h"

using namespace quavis;

Context::Context() {
  this->InitializeVkInstance();
  this->InitializeVkPhysicalDevice();
  this->InitializeVkLogicalDevice();
  this->InitializeVkShaderModules();
  this->InitializeVkRenderPass();
  this->InitializeVkGraphicsPipelineLayout();
  this->InitializeVkGraphicsPipeline();
  this->InitializeVkCommandPool();
  this->InitializeVkMemory();
  this->InitializeVkCommandBuffers();
  this->VkDraw();
}

Context::~Context() {
  debug::handleVkResult(vkDeviceWaitIdle(this->vk_logical_device_));

  // free all allocated memory
  vkFreeMemory(this->vk_logical_device_, this->vk_color_image_memory_, nullptr);
  vkFreeMemory(this->vk_logical_device_, this->vk_stencil_image_memory_, nullptr);

  // destroy images
  vkDestroyImage(this->vk_logical_device_, this->vk_color_image_, nullptr);
  vkDestroyImage(this->vk_logical_device_, this->vk_stencil_image_, nullptr);

  // destroy image views
  vkDestroyImageView(this->vk_logical_device_, this->vk_color_imageview_, nullptr);
  vkDestroyImageView(this->vk_logical_device_, this->vk_stencil_imageview_, nullptr);

  // destroy framebuffer
  vkDestroyFramebuffer(this->vk_logical_device_, this->vk_graphics_framebuffer_, nullptr);

  // destroy semaphores
  vkDestroySemaphore(this->vk_logical_device_, this->vk_render_semaphore_, nullptr);
  vkDestroySemaphore(this->vk_logical_device_, this->vk_render_finished_semaphore_, nullptr);

  // free command buffer
  vkFreeCommandBuffers(this->vk_logical_device_, this->vk_command_pool_, 1, &this->vk_graphics_commandbuffer_);

  // destroy command pool
  vkDestroyCommandPool(this->vk_logical_device_, this->vk_command_pool_, nullptr);

  // destroy render pass
  vkDestroyRenderPass(this->vk_logical_device_, this->vk_render_pass_, nullptr);

  // destroy pipeline
  vkDestroyPipelineLayout(this->vk_logical_device_, this->vk_pipeline_layout_, nullptr);
  vkDestroyPipeline(this->vk_logical_device_, this->vk_pipeline_, nullptr);

  // destroy shaders
  vkDestroyShaderModule(this->vk_logical_device_, this->vk_vertex_shader_, nullptr);
  vkDestroyShaderModule(this->vk_logical_device_, this->vk_fragment_shader_, nullptr);

  // destroy logical device
  vkDeviceWaitIdle(this->vk_logical_device_);
  vkDestroyDevice(this->vk_logical_device_, nullptr);

  // destroy instance
  vkDestroyInstance(this->vk_instance_, nullptr);
}

void Context::InitializeVkInstance() {
  uint32_t version = VK_MAKE_VERSION(VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);

  VkApplicationInfo vkApplicationInfo = VkApplicationInfo {
    VK_STRUCTURE_TYPE_APPLICATION_INFO, // type (see documentation)
    nullptr, // next structure (see documentation)
    "Quavis", // application name
    version, // quavis version
    "Quavis", // engine name
    version, // engine version,
    VK_API_VERSION_1_0 // vk version
  };

  // TODO: Check if debug during instance creation and add glfw extension
  VkInstanceCreateInfo vkInstanceCreateInfo = VkInstanceCreateInfo {
    VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, // type (see documentation)
    nullptr, // next structure (see documentation)
    0, // flags, reserver by vulkan api for future api versions
    &vkApplicationInfo, // application info (see aboive)
    this->vk_validation_layers_.size(), // number of layers (atm no debug/validation layers used)
    this->vk_validation_layers_.data(), // layer names
    this->vk_instance_extension_names_.size(), // number of extensions (atm no extensions used, here could be glfw)
    this->vk_instance_extension_names_.data() // extension names
  };

  // create the instance object
  debug::handleVkResult(
    vkCreateInstance(
      &vkInstanceCreateInfo, // creation info (see above)
      nullptr, // allocation handler (gives specific info on memory locations)
      &this->vk_instance_ // pointer where to store the instance
    )
  );
}

void Context::InitializeVkPhysicalDevice() {
  // check how many devices there are
  uint32_t num_devices = 0;
  vkEnumeratePhysicalDevices(
    this->vk_instance_, // the vk instance
    &num_devices, // The allocated memory for the number of devices
    nullptr // the allocated memory for the devices itself.
  );
  if (num_devices == 0)
    throw "No suitible device.";

  // get the devices
  std::vector<VkPhysicalDevice> devices(num_devices);
  vkEnumeratePhysicalDevices(
    this->vk_instance_, // the vk instance
    &num_devices, // The allocated memory for the number of devices
    devices.data() // the allocated memory for the devices itself.
  );

  // For requirement checking we add a set from which non-suitible devices are
  // being removed on the fly
  std::set<VkPhysicalDevice> devices_set(devices.begin(), devices.end());

  // Remove all devices that do not have graphics or compute queue family
  // TODO: Check devices for extension support
  std::set<VkPhysicalDevice>::iterator device_it;
  for (device_it = devices_set.begin(); device_it != devices_set.end();) {

    /////////////////// BEGIN LAYER CHECK
    uint32_t num_layers;
    vkEnumerateInstanceLayerProperties(&num_layers, nullptr);

    std::vector<VkLayerProperties> available_layers(num_layers);
    vkEnumerateInstanceLayerProperties(&num_layers, available_layers.data());

    bool all_found = true;
    for (const char* layerName : this->vk_validation_layers_) {
        bool layerFound = false;

        for (const auto& layerProperties : available_layers) {
            std::cout << layerProperties.layerName << std::endl;
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            device_it = devices_set.erase(device_it);
            all_found = false;
            break;
        }
    }

    if (!all_found) {
      continue;
    }

    /////////////////// BEGIN CHECK QUEUE FAMILIES
    bool has_graphics_queue = false;
    bool has_compute_queue = false;
    bool has_transfer_queue = false;

    uint32_t num_queue_families = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(
      *device_it, // the vk device
      &num_queue_families, // the allocated memory for the number of families
      nullptr // the allocated memory for the queue family properties
    );


    // No queues
    if (num_queue_families == 0) {
      device_it = devices_set.erase(device_it); // no queue families
      continue;
    }

    // get queues
    std::vector<VkQueueFamilyProperties> queue_families(num_queue_families);
    vkGetPhysicalDeviceQueueFamilyProperties(
      *device_it, // the vk device
      &num_queue_families, // the allocated memory for the number of families
      queue_families.data() // the allocated memory for the queue family properties
    );

    for (VkQueueFamilyProperties queue_family : queue_families) {
      if (!has_graphics_queue)
        has_graphics_queue = queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT;
      if (!has_compute_queue)
        has_compute_queue = queue_family.queueFlags & VK_QUEUE_COMPUTE_BIT;
      if (!has_transfer_queue)
        has_transfer_queue = queue_family.queueFlags & VK_QUEUE_TRANSFER_BIT;
    }

    if (!has_graphics_queue || !has_compute_queue || !has_transfer_queue) {
      device_it = devices_set.erase(device_it);
      continue;
    }


    /////////////////// BEGIN CHECK EXTENSION SUPPORT
    // get number of supported extension in device
    uint32_t extension_count;
    vkEnumerateDeviceExtensionProperties(
      *device_it, // the physical device
      this->vk_validation_layers_[0], // the layer name
      &extension_count, // the allocated memory for the number of extensions
      nullptr // the allocated memory for the extension properties
    );

    // get device's supported extensions
    std::vector<VkExtensionProperties> available_extensions(extension_count);
    vkEnumerateDeviceExtensionProperties(
      *device_it, // the physical device
      this->vk_validation_layers_[0], // the layer name
      &extension_count, // the allocated memory for the number of extensions
      available_extensions.data() // the allocated memory for the extension properties
    );

    std::set<std::string> required_extensions(this->vk_logical_device_extension_names_.begin(), vk_logical_device_extension_names_.end());
    for (VkExtensionProperties extension : available_extensions) {
        required_extensions.erase(extension.extensionName);
    }

    // check all requirements
    if (!required_extensions.empty()) {
      device_it = devices_set.erase(device_it);
      continue;
    }
    /////////////////// END REQUIREMENT CHECK
    device_it++;
  }

  // Check if there are devices meeting the requirements
  // if so, pick the first one (random)
  if (devices_set.size() == 0)
    throw "No suitible device";
  else
    this->vk_physical_device_ = *devices_set.begin();
}

void Context::InitializeVkLogicalDevice() {
  // get graphics and compute queue family index for selected device

  uint32_t num_queue_families = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(
    this->vk_physical_device_, // the vk device
    &num_queue_families, // the allocated memory for the number of families
    nullptr // the allocated memory for the queue family properties
  );

  // get queues
  std::vector<VkQueueFamilyProperties> queue_families(num_queue_families);
  vkGetPhysicalDeviceQueueFamilyProperties(
    this->vk_physical_device_, // the vk device
    &num_queue_families, // the allocated memory for the number of families
    queue_families.data() // the allocated memory for the queue family properties
  );

  // get index of suitible queue family
  // TODO: Maybe separate graphics, compute and transfer queue families
  this->queue_family_index_ = 0;
  for (VkQueueFamilyProperties queue_family : queue_families) {
    uint32_t requirements = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;
    if (queue_family.queueFlags & requirements)
      break;
    this->queue_family_index_++;
  }

  // Create graphics queue metadata
  // TODO: Seperate queue construction if multiple families are required
  float queue_family_priorities[] = { 1.0f, 1.0f, 1.0f };
  VkDeviceQueueCreateInfo queue_create_info {
    VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, // sType (see documentation)
    nullptr, // next structure (see documentation)
    0, // queue flags MUST be 0 (see documentation)
    this->queue_family_index_, // queue family
    3, // number of queues: Graphics, Compute, Transfer // TODO: Maybe add more queues for more parallelism (?)
    queue_family_priorities // queue priority
  };

  // Specify device features
  // TODO: Specify device features
  VkPhysicalDeviceFeatures device_features = {};

  // Create lgocial device metadata
  VkDeviceCreateInfo device_create_info = {
    VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, // sType (see documentation)
    nullptr, // next (see documentation)
    0, // flags (see documentation)
    1, // number of queue create info objects
    &queue_create_info, // queue meta data
    0, // deprecated & ignored
    nullptr, // depcrecated & ignored
    (uint32_t)this->vk_logical_device_extension_names_.size(), // enabled extensions
    this->vk_logical_device_extension_names_.data(), // extension names
    &device_features // enabled device features
  };

  // create logical device
  debug::handleVkResult(
    vkCreateDevice(
      this->vk_physical_device_, // the physical device
      &device_create_info, // logical device metadata
      nullptr, // allocation callback (see documentation)
      &this->vk_logical_device_ // the allocated memory for the logical device
    )
  );

  // get graphics queue
  vkGetDeviceQueue(
    this->vk_logical_device_, // the logical device
    this->queue_family_index_, // the queue family from which we want the queue
    0, // the index of the queue we want < NUM_QUEUES_IN_FAMILY
    &this->vk_queue_graphics_ // the allocated memory for the queue
  );

  // get compute queue
  vkGetDeviceQueue(
    this->vk_logical_device_, // the logical device
    this->queue_family_index_, // the queue family from which we want the queue
    1, // the index of the queue we want < NUM_QUEUES_IN_FAMILY
    &this->vk_queue_compute_ // the allocated memory for the queue
  );

  // get transfer queue
  vkGetDeviceQueue(
    this->vk_logical_device_, // the logical device
    this->queue_family_index_, // the queue family from which we want the queue
    2, // the index of the queue we want < NUM_QUEUES_IN_FAMILY
    &this->vk_queue_transfer_ // the allocated memory for the queue
  );
}

void Context::InitializeVkShaderModules() {
  // TODO: Add better mechanism for shader loading

  // create vertex shader
  VkShaderModuleCreateInfo vertex_shader_info = {
    VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, // type (see documentation)
    nullptr, // next (see documentation, must be null)
    0, // flags (see documentation, must be 0)
    src_shaders_vert_spv_len, // vertex shader size
    (uint32_t*)src_shaders_vert_spv // vertex shader code
  };

  debug::handleVkResult(
    vkCreateShaderModule(
      this->vk_logical_device_, // the logical device
      &vertex_shader_info, // shader meta data
      nullptr, // allocation callback (see documentation)
      &this->vk_vertex_shader_ // the allocated memory for the logical device
    )
  );

  // create fragment shader
  VkShaderModuleCreateInfo fragment_shader_info = {
    VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, // type (see documentation)
    nullptr, // next (see documentation, must be null)
    0, // flags (see documentation, must be 0)
    src_shaders_frag_spv_len, // fragment shader size
    (uint32_t*)src_shaders_frag_spv // fragment shader code
  };

  debug::handleVkResult(
    vkCreateShaderModule(
      this->vk_logical_device_, // the logical device
      &fragment_shader_info, // shader meta data
      nullptr, // allocation callback (see documentation)
      &this->vk_fragment_shader_ // the allocated memory for the logical device
    )
  );
}

void Context::InitializeVkRenderPass() {
  // create attachment descriptions for color / stencil
  VkAttachmentDescription color_attachment_description = {
    0, // flags (see documentation, 1 option)
    this->color_format_, // color format
    VK_SAMPLE_COUNT_1_BIT, // num samples per fragment
    VK_ATTACHMENT_LOAD_OP_CLEAR, // operation when loading
    VK_ATTACHMENT_STORE_OP_STORE, // operation when storing
    VK_ATTACHMENT_LOAD_OP_DONT_CARE, // stencil operation when loading
    VK_ATTACHMENT_STORE_OP_DONT_CARE, // stencil operation when storing
    VK_IMAGE_LAYOUT_UNDEFINED, // initial layout
    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL // final layout (optimal for memory transfer)
  };

  VkAttachmentDescription stencil_attachment_description = {
    0, // flags (see documentation, 1 option)
    this->stencil_format_, // color format
    VK_SAMPLE_COUNT_1_BIT, // num samples per fragment
    VK_ATTACHMENT_LOAD_OP_CLEAR, // operation when loading
    VK_ATTACHMENT_STORE_OP_STORE, // operation when storing
    VK_ATTACHMENT_LOAD_OP_DONT_CARE, // stencil operation when loading
    VK_ATTACHMENT_STORE_OP_DONT_CARE, // stencil operation when storing
    VK_IMAGE_LAYOUT_UNDEFINED, // initial layout
    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL // final layout (optimal for memory transfer)
  };

  VkAttachmentDescription attachment_descriptions[] = {
    color_attachment_description
  };

  // create attachment refernces for color / stencil
  VkAttachmentReference color_attachment_reference = {
    0, // index
    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL // layout
  };

  VkAttachmentReference stencil_attachment_reference = {
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
    nullptr, // stencil attachment
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
    1, // attachment count
    attachment_descriptions, // attachment descriptions
    1, // subpass count
    &subpass_description, // subpass
    1, // dependency count between subpasses
    &dependency // dependencies
  };

  debug::handleVkResult(
    vkCreateRenderPass(
      this->vk_logical_device_,
      &render_pass_info,
      nullptr,
      &this->vk_render_pass_
    )
  );
}

void Context::InitializeVkGraphicsPipelineLayout() {
  // Define Pipeline layout
  VkPipelineLayoutCreateInfo pipeline_layout_info = {
    VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, // sType
    nullptr, // next (see documentation, must be null)
    0, // flags (see documentation, must be 0)
    0, // layout count
    nullptr, // layouts
    0, // push constant range count
    nullptr // push constant ranges
  };

  // Create pipeline layout
  debug::handleVkResult(
    vkCreatePipelineLayout(
      this->vk_logical_device_,
      &pipeline_layout_info,
      nullptr,
      &this->vk_pipeline_layout_
    )
  );
}

void Context::InitializeVkGraphicsPipeline() {
  // create pipeline shader stages
  VkPipelineShaderStageCreateInfo vertex_shader_stage_info = {
    VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, // sType (see documentation)
    nullptr, // next (see documentation, must be null)
    0, // flags (see documentation, must be 0)
    VK_SHADER_STAGE_VERTEX_BIT, // stage flag
    this->vk_vertex_shader_, // shader module
    "main", // the pipeline's name
    nullptr // VkSpecializationInfo (see documentation)
  };

  VkPipelineShaderStageCreateInfo fragment_shader_stage_info = {
    VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, // sType (see documentation)
    nullptr, // next (see documentation, must be null)
    0, // flags (see documentation, must be 0)
    VK_SHADER_STAGE_FRAGMENT_BIT, // stage flag
    this->vk_fragment_shader_, // shader module
    "main", // the pipeline's name
    nullptr // VkSpecializationInfo (see documentation)
  };

  VkPipelineShaderStageCreateInfo shader_stages[] = {vertex_shader_stage_info, fragment_shader_stage_info};

  // Define input loading
  // TODO: Load vertices correctly
  VkPipelineVertexInputStateCreateInfo vertex_input_info {
    VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO, // sType
    nullptr, // next (see documentation, must be null)
    0, // flags (see documentation, must be 0)
    0, // binding description count (spacing of data etc.)
    nullptr, // binding descriptions, here we don't load vertices atm
    0, // attribute description count (types passed to vertex shader etc.)
    nullptr // attribute descriptions
  };

  VkPipelineInputAssemblyStateCreateInfo input_assembly_info {
    VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, // sType
    nullptr, // pNext (see documentation, must be null)
    0, // flags (see documentation, must be 0)
    VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, // topology of vertices
    VK_FALSE // whether there should be a special vertex index to reassemble
  };

  // Define viewport
  VkViewport viewport = {
    0.0f, // upper left corner x
    0.0f, // upper left corner y
    (float)this->render_width_, // width
    (float)this->render_height_, // height
    0.0f, // min depth
    1.0f // max depth
  };

  VkRect2D scissor = {
    {0, 0}, // offset (casted to VkOffset2D)
    {this->render_width_, this->render_height_} // extent (casted to VkExtent2D)
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
    VK_CULL_MODE_BACK_BIT, // discard one of the two faces of a polygon
    VK_FRONT_FACE_CLOCKWISE, // clockwise = front
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
    VK_COMPARE_OP_LESS, // comparison operation // TODO: Check if depth comparator is correct
    VK_FALSE, // depth bound test
    VK_FALSE, // stencil test
    {}, // front stencil op state
    {}, // back stencil op state
    0.0f, // min depth // TODO: Is min depth / max depth correct here?
    1.0f // max depth
  };

  // Define pipeline info
  VkGraphicsPipelineCreateInfo pipeline_info = {
    VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO, // sType
    nullptr, // next (see documentation, must be null)
    0, // pipeline create flags (have no child pipelines, so don't care)
    2, // number of stages (we have 2 shaders for now)
    shader_stages, // shader stage create infos
    &vertex_input_info, // vertex input info
    &input_assembly_info, // inpt assembly info
    nullptr, // tesselation info
    &viewport_info, // viewport info
    &rasterizer_info, // rasterization info
    &multisampling_info, // multisampling info
    &depth_stencil_info, // depth stencil info
    &color_blend_info, // blending info
    nullptr, // dynamic states info (e.g. window size changes or so)
    this->vk_pipeline_layout_, // pipeline layout
    this->vk_render_pass_, // render pass
    0, // subpass index for this pipeline (we only have 1)
    VK_NULL_HANDLE, // parent pipeline
    -1 // parent pipeline index
  };

  debug::handleVkResult(
    vkCreateGraphicsPipelines(
      this->vk_logical_device_, // logical device
      VK_NULL_HANDLE, // pipeline cache // TODO: Add pipeline cache (?)
      1, // pipeline count
      &pipeline_info, // pipeline infos
      nullptr, // allocation callback
      &this->vk_pipeline_ // allocated memory for the pipeline
    )
  );
}

void Context::InitializeVkCommandPool() {
  VkCommandPoolCreateInfo command_pool_info = {
    VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, // sType
    nullptr,// pNext (see documentation, must be null)
    0, // flags (see documentation, must be 0)
    this->queue_family_index_ // the queue family
  };

  debug::handleVkResult(
    vkCreateCommandPool(
      this->vk_logical_device_, // the logical device
      &command_pool_info, // info
      nullptr, // allocation callback
      &this->vk_command_pool_ // the allocated memory
    )
  );
}

void Context::InitializeVkMemory() {
  ///////////////// COLOR IMAGE
  // create color image
  VkImageCreateInfo color_image_info = {
    VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, // sType,
    nullptr, // pNext (see documentation, must be null)
    0, // image flags
    VK_IMAGE_TYPE_2D, // image type
    this->color_format_, // image format
    {this->render_width_, this->render_height_, 1}, // image extent
    1, // level of detail = 1
    1, // layers = 1
    VK_SAMPLE_COUNT_1_BIT, // image sampling per pixel
    VK_IMAGE_TILING_OPTIMAL, // optimal tiling
    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, // used for color
    VK_SHARING_MODE_EXCLUSIVE, // sharing between queue families
    1, // number queue families
    &this->queue_family_index_, // queue family index
    VK_IMAGE_LAYOUT_UNDEFINED // initial layout
  };

  debug::handleVkResult(
    vkCreateImage(
      this->vk_logical_device_, // the logical device
      &color_image_info, // the image info
      nullptr, // allocation
      &this->vk_color_image_ // memory allocated for image object
    )
  );

  // allocate color image memory
  VkMemoryRequirements color_memory_requirements;
  vkGetImageMemoryRequirements(
    this->vk_logical_device_,
    this->vk_color_image_,
    &color_memory_requirements
  );

  // get the first set bit in the type-bits. This bit is at the index position
  // of a supported memory type in the physical device
  uint32_t color_memory_type = 0;
  uint32_t color_memory_type_bit = 1;
  uint32_t color_memory_type_bits = color_memory_requirements.memoryTypeBits;
  while ((color_memory_type_bit & color_memory_type_bits) == 0) {
    color_memory_type_bit *= 2;
    color_memory_type++;
  }

  VkMemoryAllocateInfo color_allocation_info = {
    VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, // sType
    nullptr, // pNext (see documentation, must be null)
    color_memory_requirements.size, // the memory size
    color_memory_type // the memory type index
  };

  debug::handleVkResult(
    vkAllocateMemory(
      this->vk_logical_device_, // the logical devcie
      &color_allocation_info, // the allocation info
      nullptr, // allocation callback
      &this->vk_color_image_memory_ // allocated memory for memory object
    )
  );

  debug::handleVkResult(
    vkBindImageMemory(
      this->vk_logical_device_, // the logical device
      this->vk_color_image_, // the image
      this->vk_color_image_memory_, // the image memory
      0 // the offset in the memory
    )
  );

  ///////////////// STENCIL IMAGE
  // create stencil image
  VkImageCreateInfo stencil_image_info = {
    VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, // sType,
    nullptr, // pNext (see documentation, must be null)
    0, // image flags
    VK_IMAGE_TYPE_2D, // image type
    this->stencil_format_, // image format
    {this->render_width_, this->render_height_, 1}, // image extent
    1, // level of detail = 1
    1, // layers = 1
    VK_SAMPLE_COUNT_1_BIT, // image sampling per pixel
    VK_IMAGE_TILING_OPTIMAL, // linear tiling
    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, // used for color
    VK_SHARING_MODE_EXCLUSIVE, // sharing between queue families
    1, // number queue families
    &this->queue_family_index_, // queue family index
    VK_IMAGE_LAYOUT_UNDEFINED // initial layout // TODO: !!!Find out whether this has to be transisition first
  };

  debug::handleVkResult(
    vkCreateImage(
      this->vk_logical_device_, // the logical device
      &stencil_image_info, // the image info
      nullptr, // allocation
      &this->vk_stencil_image_ // memory allocated for image object
    )
  );

  // allocate stencil image memory
  VkMemoryRequirements stencil_memory_requirements;
  vkGetImageMemoryRequirements(
    this->vk_logical_device_,
    this->vk_stencil_image_,
    &stencil_memory_requirements
  );

  // get the first set bit in the type-bits. This bit is at the index position
  // of a supported memory type in the physical device
  uint32_t stencil_memory_type = 0;
  uint32_t stencil_memory_type_bit = 1;
  uint32_t stencil_memory_type_bits = stencil_memory_requirements.memoryTypeBits;
  while ((stencil_memory_type_bit & stencil_memory_type_bits) == 0) {
    stencil_memory_type_bit *= 2;
    stencil_memory_type++;
  }

  VkMemoryAllocateInfo stencil_allocation_info = {
    VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, // sType
    nullptr, // pNext (see documentation, must be null)
    stencil_memory_requirements.size, // the memory size
    stencil_memory_type // the memory type index
  };

  debug::handleVkResult(
    vkAllocateMemory(
      this->vk_logical_device_, // the logical devcie
      &stencil_allocation_info, // the allocation info
      nullptr, // allocation callback
      &this->vk_stencil_image_memory_ // allocated memory for memory object
    )
  );

  debug::handleVkResult(
    vkBindImageMemory(
      this->vk_logical_device_, // the logical device
      this->vk_stencil_image_, // the image
      this->vk_stencil_image_memory_, // the image memory
      0 // the offset in the memory
    )
  );

  // TODO: Allocate image memory with linear tiling to copy the original ones
  // for host usage (linear tiling is incompatible with color / depth attachment)
  // the new memory must reside in host visible, host coherent memory and have a
  // compatible format

  // Create Image views
  VkImageViewCreateInfo color_imageview_info = {
    VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, // sType
    nullptr,// pNext (see documentation, must be null)
    0, // flags (see documentation, must be 0)
    this->vk_color_image_, // the image
    VK_IMAGE_VIEW_TYPE_2D, // the view type
    this->color_format_, // the color format
    {}, // color component mapping
    { // VkImageSubresourceRange (what's in the image)
      VK_IMAGE_ASPECT_COLOR_BIT, // aspect flag
      0, // base level
      1, // level count
      0, // base layer
      1 // layer count
    }
  };

  debug::handleVkResult(
    vkCreateImageView(
      this->vk_logical_device_, // the logical device
      &color_imageview_info, // info
      nullptr, // allocation callback
      &this->vk_color_imageview_ // the offset in the memory
    )
  );

  VkImageViewCreateInfo stencil_imageview_info = {
    VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, // sType
    nullptr,// pNext (see documentation, must be null)
    0, // flags (see documentation, must be 0)
    this->vk_stencil_image_, // the image
    VK_IMAGE_VIEW_TYPE_2D, // the view type
    this->stencil_format_, // the stencil format
    {}, // stencil component mapping
    { // VkImageSubresourceRange (what's in the image)
      VK_IMAGE_ASPECT_DEPTH_BIT, // aspect flag
      0, // base level
      1, // level count
      0, // base layer
      1 // layer count
    }
  };

  debug::handleVkResult(
    vkCreateImageView(
      this->vk_logical_device_, // the logical device
      &stencil_imageview_info, // info
      nullptr, // allocation callback
      &this->vk_stencil_imageview_ // the allocated memory
    )
  );

  // Create Framebuffers for color & stencil (color & depth)
  std::array<VkImageView, 1> attachments = {
    this->vk_color_imageview_
  };

  VkFramebufferCreateInfo framebuffer_info = {
    VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, // sType
    nullptr,// pNext (see documentation, must be null)
    0, // flags (see documentation, must be 0)
    this->vk_render_pass_, // render pass
    attachments.size(), // attachment count
    attachments.data(), // attachments
    this->render_width_, // width
    this->render_height_, // height
    1 // layer count
  };

  debug::handleVkResult(
    vkCreateFramebuffer(
      this->vk_logical_device_, // the logical device
      &framebuffer_info, // info
      nullptr, // allocation callback
      &this->vk_graphics_framebuffer_ // the allocated memory
    )
  );

  // create command buffers
  VkCommandBufferAllocateInfo graphics_command_buffer_info = {
    VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, // sType
    nullptr, // pNext (see documentation, must be null)
    this->vk_command_pool_, // the command pool
    VK_COMMAND_BUFFER_LEVEL_PRIMARY, // can be submitted to the queue
    1 // number of command buffers
  };

  debug::handleVkResult(
    vkAllocateCommandBuffers(
      this->vk_logical_device_, // the logical device
      &graphics_command_buffer_info, // info
      &this->vk_graphics_commandbuffer_ // allocated memory
    )
  );
}

void Context::InitializeVkCommandBuffers() {
  VkCommandBufferBeginInfo command_buffer_begin_info = {
    VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, // sType
    nullptr, // pNext (see documentation, must be null)
    VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT, // can be submitted and run simultaniously
    nullptr // VkCommandBufferInheritanceInfo (we don't need it)
  };

  debug::handleVkResult(
    vkBeginCommandBuffer(
      this->vk_graphics_commandbuffer_,
      &command_buffer_begin_info
    )
  );

  VkClearValue clear_value[] = {{0.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f, 1.0f}};

  VkRenderPassBeginInfo render_pass_info = {
    VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO, // sType
    nullptr, // pNext (see documentation, must be null)
    this->vk_render_pass_, // render pass
    this->vk_graphics_framebuffer_, // framebuffer
    {{0,0}, {this->render_width_, this->render_height_}}, // render area (VkRect2D)
    2, // number of clear values
    clear_value // clear values
  };

  vkCmdBeginRenderPass(
    this->vk_graphics_commandbuffer_, // command buffer
    &render_pass_info, // render pass info
    VK_SUBPASS_CONTENTS_INLINE // store contents in primary command buffer
  );

  // bind graphics pipeline
  vkCmdBindPipeline(
    this->vk_graphics_commandbuffer_, // command buffer
    VK_PIPELINE_BIND_POINT_GRAPHICS, // pipeline type
    this->vk_pipeline_ // graphics pipeline
  );

  // draw
  vkCmdDraw(
    this->vk_graphics_commandbuffer_, // command buffer
    3, // num vertices // TODO
    1, // num instances // TODO
    0, // first vertex index
    0 // first instance ID
  );

  vkCmdEndRenderPass(this->vk_graphics_commandbuffer_);

  debug::handleVkResult(
    vkEndCommandBuffer(
      this->vk_graphics_commandbuffer_
    )
  );
}

void Context::VkDraw() {
  // create semaphore
  VkSemaphoreCreateInfo render_semaphore_info = {
    VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, // sType,
    nullptr, // next (see documentaton, must be null)
    0, // flags (see documentation, must be 0)
  };

  debug::handleVkResult(
    vkCreateSemaphore(
      this->vk_logical_device_, // the logical device
      &render_semaphore_info, // semaphore info
      nullptr, // allocation callback
      &this->vk_render_semaphore_ // allocated memory
    )
  );

  debug::handleVkResult(
    vkCreateSemaphore(
      this->vk_logical_device_, // the logical device
      &render_semaphore_info, // semaphore info
      nullptr, // allocation callback
      &this->vk_render_finished_semaphore_ // allocated memory
    )
  );

  // submit the graphics command buffer
  VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

  VkSubmitInfo submit_info = {
    VK_STRUCTURE_TYPE_SUBMIT_INFO, // sType,
    nullptr, // next (see documentaton, must be null)
    0, // wait semaphore count
    nullptr, // semaphore to wait for
    wait_stages, // stage until next semaphore is triggered
    1, //
    &this->vk_graphics_commandbuffer_,
    0,
    nullptr
  };

  debug::handleVkResult(
    vkQueueSubmit(
      this->vk_queue_graphics_, // queue
      1, // num infos
      &submit_info, // info
      VK_NULL_HANDLE // fence (we don't need it)
    )
  );
}
