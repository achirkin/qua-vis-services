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
}

Context::~Context() {
  // destroy render pass
  vkDestroyRenderPass(this->vk_logical_device_, this->vk_render_pass_, nullptr);

  // destroy pipeline
  vkDestroyPipelineLayout(this->vk_logical_device_, this->vk_pipeline_layout_, nullptr);

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
    0, // number of layers (atm no debug/validation layers used)
    nullptr, // layer names
    0, // number of extensions (atm no extensions used, here could be glfw)
    nullptr // extension names
  };

  // create the instance object
  vk::handleVkResult(
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
      nullptr, // the layer name
      &extension_count, // the allocated memory for the number of extensions
      nullptr // the allocated memory for the extension properties
    );

    // get device's supported extensions
    std::vector<VkExtensionProperties> available_extensions(extension_count);
    vkEnumerateDeviceExtensionProperties(
      *device_it, // the physical device
      nullptr, // the layer name
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
  uint32_t queue_family_index = 0;
  for (VkQueueFamilyProperties queue_family : queue_families) {
    uint32_t requirements = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;
    if (queue_family.queueFlags & requirements)
      break;
    queue_family_index++;
  }

  // Create graphics queue metadata
  // TODO: Seperate queue construction if multiple families are required
  float queue_family_priorities[] = { 1.0f, 1.0f, 1.0f };
  VkDeviceQueueCreateInfo queue_create_info {
    VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, // sType (see documentation)
    nullptr, // next structure (see documentation)
    0, // queue flags MUST be 0 (see documentation)
    queue_family_index, // queue family
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
  vk::handleVkResult(
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
    queue_family_index, // the queue family from which we want the queue
    0, // the index of the queue we want < NUM_QUEUES_IN_FAMILY
    &this->vk_queue_graphics_ // the allocated memory for the queue
  );

  // get compute queue
  vkGetDeviceQueue(
    this->vk_logical_device_, // the logical device
    queue_family_index, // the queue family from which we want the queue
    1, // the index of the queue we want < NUM_QUEUES_IN_FAMILY
    &this->vk_queue_compute_ // the allocated memory for the queue
  );

  // get transfer queue
  vkGetDeviceQueue(
    this->vk_logical_device_, // the logical device
    queue_family_index, // the queue family from which we want the queue
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
    this->vertex_shader_code_.size(), // vertex shader size
    (uint32_t*)this->vertex_shader_code_.data() // vertex shader code
  };

  vk::handleVkResult(
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
    this->fragment_shader_code_.size(), // fragment shader size
    (uint32_t*)this->fragment_shader_code_.data() // fragment shader code
  };

  vk::handleVkResult(
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
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL // final layout (optimal for memory transfer)
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
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL // final layout (optimal for memory transfer)
  };

  VkAttachmentDescription attachment_descriptions[] = {
    color_attachment_description,
    stencil_attachment_description
  };

  // create attachment refernces for color / stencil
  VkAttachmentReference color_attachment_reference = {
    0, // index
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL // layout
  };

  VkAttachmentReference stencil_attachment_reference = {
    1, // index
    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL // layout
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
    &stencil_attachment_reference, // stencil attachment
    0, // preserved attachment count
    nullptr // preserved attachments
  };

  // create render pass
  VkRenderPassCreateInfo render_pass_info = {
    VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO, // sType
    nullptr, // next (see documentation, must be null)
    0, // flags
    2, // attachment count
    attachment_descriptions, // attachment descriptions
    1, // subpass count
    &subpass_description, // subpass
    0, // dependency count between subpasses
    nullptr // dependencies
  };

  vk::handleVkResult(
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
  vk::handleVkResult(
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

  vk::handleVkResult(
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
