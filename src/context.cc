#include "quavis/quavis.h"
#include <chrono>
#include <cfloat>
#include <unordered_map>
#include <unordered_set>

using namespace quavis;

Context::Context(std::string shader_name) {
  this->shader_name_ = shader_name;

  this->InitializeVkInstance();
  this->InitializeVkPhysicalDevice();
  this->InitializeVkLogicalDevice();
  this->InitializeVkShaderModules();
  this->InitializeVkRenderPass();
  this->InitializeVkDescriptorPool();
  this->InitializeVkDescriptorSetLayout();
  this->InitializeVkGraphicsPipelineLayout();
  this->InitializeVkComputePipelineLayout();
  this->InitializeVkGraphicsPipeline();
  this->InitializeVkComputePipeline();
}

std::vector<float> Context::Parse(std::string contents, std::vector<vec3> analysispoints, float alpha_max, float r_max) {
  this->uniform_.alpha_max = alpha_max;
  this->uniform_.r_max = r_max;
  std::vector<vec3> points = geojson::parse(contents);
  std::unordered_map<Vertex, int> vertex_map = {};
  vertices_ = std::vector<Vertex>();
  for (uint32_t i = 0; i < points.size(); i++) {
    Vertex vertex = {points[i], {255,255,255}};
    if (vertex_map.count(vertex) == 0) {
      vertex_map[vertex] = vertices_.size();
      vertices_.push_back(vertex);
    }
    indices_.push_back(vertex_map[vertex]);
  }

  this->InitializeVkMemory();
  this->InitializeVkImageLayouts();

  std::vector<vec3> observation_points = analysispoints;

  //// Create a list of vertices that lie are in some triangle
  //std::unordered_set<size_t> ignore = {};
  //for (size_t o = 0; o < observation_points.size(); o++) {
  //  for (size_t i = 0; i < indices_.size(); i+=3) {
  //    vec2 p0, p1, p2;
  //    p0 = {vertices_[indices_[i]].pos.x, vertices_[indices_[i]].pos.y};
  //    p1 = {vertices_[indices_[i+1]].pos.x, vertices_[indices_[i+1]].pos.y};
  //    p2 = {vertices_[indices_[i+2]].pos.x, vertices_[indices_[i+2]].pos.y};
  //  }
  //}

  this->SubmitVertexData();
  this->SubmitIndexData();
  this->SubmitUniformData();

  VkDescriptorSetLayout layouts[] = {this->vk_graphics_descriptor_set_layout_};
  this->CreateGraphicsDescriptorSet(layouts, &this->vk_graphics_descriptor_set_);
  this->UpdateGraphicsDescriptorSet(sizeof(UniformBufferObject), this->vk_uniform_buffer_, &this->vk_graphics_descriptor_set_);
  this->InitializeVkGraphicsCommandBuffers();

  VkFenceCreateInfo fenceCreateInfo = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr, 0x00000001};
  debug::handleVkResult(vkCreateFence(this->vk_logical_device_,&fenceCreateInfo,nullptr,&this->vk_compute_fence_));
  this->CreateComputeDescriptorSets();
  this->UpdateComputeDescriptorSets();
  this->InitializeVkComputeCommandBuffers();

  // MAGIIC
  std::vector<float> results(observation_points.size());
  for (size_t i = 0; i < observation_points.size(); i++) {
    this->uniform_.observation_point = observation_points[i];
    this->SubmitUniformData();
    vkQueueWaitIdle(this->vk_queue_graphics_);
    this->VkDraw();
    this->ResetResult();
    vkQueueWaitIdle(this->vk_queue_graphics_);
    this->VkCompute();
    vkQueueWaitIdle(this->vk_queue_compute_);
    results[i] = *(float*)this->RetrieveResult();

    if (imagesRequired) {
      RetrieveRenderImage(i);
      RetrieveDepthImage(i);
      //RetrieveComputeImage(i);
    }
  }
  return results;
}

Context::~Context() {
  debug::handleVkResult(vkDeviceWaitIdle(this->vk_logical_device_));

  // free all allocated memory
  vkFreeMemory(this->vk_logical_device_, this->vk_color_image_memory_, nullptr);
  vkFreeMemory(this->vk_logical_device_, this->vk_depth_stencil_image_memory_, nullptr);
  vkFreeMemory(this->vk_logical_device_, this->vk_color_staging_image_memory_, nullptr);
  vkFreeMemory(this->vk_logical_device_, this->vk_depth_stencil_staging_image_memory_, nullptr);
  vkFreeMemory(this->vk_logical_device_, this->vk_vertex_buffer_memory_, nullptr);
  vkFreeMemory(this->vk_logical_device_, this->vk_vertex_staging_buffer_memory_, nullptr);
  vkFreeMemory(this->vk_logical_device_, this->vk_index_buffer_memory_, nullptr);
  vkFreeMemory(this->vk_logical_device_, this->vk_index_staging_buffer_memory_, nullptr);
  vkFreeMemory(this->vk_logical_device_, this->vk_uniform_buffer_memory_, nullptr);
  vkFreeMemory(this->vk_logical_device_, this->vk_uniform_staging_buffer_memory_, nullptr);
  vkFreeMemory(this->vk_logical_device_, this->vk_compute_tmp_buffer_memory_, nullptr);
  vkFreeMemory(this->vk_logical_device_, this->vk_compute_buffer_memory_, nullptr);
  vkFreeMemory(this->vk_logical_device_, this->vk_compute_staging_buffer_memory_, nullptr);

  // destroy vertex buffer
  vkDestroyBuffer(this->vk_logical_device_, this->vk_vertex_buffer_, nullptr);
  vkDestroyBuffer(this->vk_logical_device_, this->vk_vertex_staging_buffer_, nullptr);
  vkDestroyBuffer(this->vk_logical_device_, this->vk_index_buffer_, nullptr);
  vkDestroyBuffer(this->vk_logical_device_, this->vk_index_staging_buffer_, nullptr);
  vkDestroyBuffer(this->vk_logical_device_, this->vk_uniform_buffer_, nullptr);
  vkDestroyBuffer(this->vk_logical_device_, this->vk_uniform_staging_buffer_, nullptr);
  vkDestroyBuffer(this->vk_logical_device_, this->vk_compute_tmp_buffer_, nullptr);
  vkDestroyBuffer(this->vk_logical_device_, this->vk_compute_buffer_, nullptr);
  vkDestroyBuffer(this->vk_logical_device_, this->vk_compute_staging_buffer_, nullptr);

  // destroy images
  vkDestroyImage(this->vk_logical_device_, this->vk_color_image_, nullptr);
  vkDestroyImage(this->vk_logical_device_, this->vk_depth_stencil_image_, nullptr);
  vkDestroyImage(this->vk_logical_device_, this->vk_color_staging_image_, nullptr);
  vkDestroyImage(this->vk_logical_device_, this->vk_depth_stencil_staging_image_, nullptr);

  // destroy image views
  vkDestroyImageView(this->vk_logical_device_, this->vk_color_imageview_, nullptr);
  vkDestroyImageView(this->vk_logical_device_, this->vk_depth_stencil_imageview_, nullptr);

  // destroy fences
  vkDestroyFence(this->vk_logical_device_, this->vk_compute_fence_, nullptr);

  // destroy framebuffer
  vkDestroyFramebuffer(this->vk_logical_device_, this->vk_graphics_framebuffer_, nullptr);

  // free command buffer
  vkFreeCommandBuffers(this->vk_logical_device_, this->vk_graphics_command_pool_, 1, &this->vk_graphics_commandbuffer_);
  vkFreeCommandBuffers(this->vk_logical_device_, this->vk_compute_command_pool_, 1, &this->vk_compute_commandbuffer_);

  // destroy command pool
  vkDestroyCommandPool(this->vk_logical_device_, this->vk_graphics_command_pool_, nullptr);
  vkDestroyCommandPool(this->vk_logical_device_, this->vk_compute_command_pool_, nullptr);

  // destroy render pass
  vkDestroyRenderPass(this->vk_logical_device_, this->vk_render_pass_, nullptr);

  // destroy descriptor set layout
  vkDestroyDescriptorSetLayout(this->vk_logical_device_, this->vk_graphics_descriptor_set_layout_, nullptr);
  vkDestroyDescriptorSetLayout(this->vk_logical_device_, this->vk_compute_descriptor_set_layout_, nullptr);
  vkDestroyDescriptorPool(this->vk_logical_device_, this->vk_descriptor_pool_, nullptr);

  // destroy pipeline
  vkDestroyPipelineLayout(this->vk_logical_device_, this->vk_graphics_pipeline_layout_, nullptr);
  vkDestroyPipeline(this->vk_logical_device_, this->vk_graphics_pipeline_, nullptr);
  vkDestroyPipelineLayout(this->vk_logical_device_, this->vk_compute_pipeline_layout_, nullptr);
  vkDestroyPipeline(this->vk_logical_device_, this->vk_compute_pipeline_, nullptr);
  vkDestroyPipeline(this->vk_logical_device_, this->vk_compute_pipeline_2_, nullptr);

  // destroy shaders
  vkDestroyShaderModule(this->vk_logical_device_, this->vk_vertex_shader_, nullptr);
  vkDestroyShaderModule(this->vk_logical_device_, this->vk_tessellation_control_shader_, nullptr);
  vkDestroyShaderModule(this->vk_logical_device_, this->vk_tessellation_evaluation_shader_, nullptr);
  vkDestroyShaderModule(this->vk_logical_device_, this->vk_geoemtry_shader_, nullptr);
  vkDestroyShaderModule(this->vk_logical_device_, this->vk_fragment_shader_, nullptr);
  vkDestroyShaderModule(this->vk_logical_device_, this->vk_compute_shader_, nullptr);
  vkDestroyShaderModule(this->vk_logical_device_, this->vk_compute_shader_2_, nullptr);

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
    (uint32_t)this->vk_validation_layers_.size(), // number of layers (atm no debug/validation layers used)
    this->vk_validation_layers_.data(), // layer names
    (uint32_t)this->vk_instance_extension_names_.size(), // number of extensions (atm no extensions used, here could be glfw)
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
  
  
  std::cout << "Number of devices: " << num_devices << std::endl;
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
    const char* validation_layer_name = nullptr;
    if (this->vk_validation_layers_.size() > 0)
      validation_layer_name = this->vk_validation_layers_[0];

    // get number of supported extension in device
    uint32_t extension_count;
    vkEnumerateDeviceExtensionProperties(
      *device_it, // the physical device
      validation_layer_name, // the layer name
      &extension_count, // the allocated memory for the number of extensions
      nullptr // the allocated memory for the extension properties
    );

    // get device's supported extensions
    std::vector<VkExtensionProperties> available_extensions(extension_count);
    vkEnumerateDeviceExtensionProperties(
      *device_it, // the physical device
      validation_layer_name, // the layer name
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

  // prefer discrete GPU.
  // other than that, use any card meeting the requirements
  VkPhysicalDevice dev;
  VkPhysicalDeviceProperties props;
  for (device_it = devices_set.begin(); device_it != devices_set.end();) {
    vkGetPhysicalDeviceProperties(*device_it, &props);
    if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
      dev = *device_it;
      break;
    }
    device_it++;
  }
  std::cout << "Number of GPUs fit: " << devices_set.size() << std::endl;
  if (devices_set.size() == 0)
    throw "No suitible device";
  else {
    std::cout << "Selected: " << props.deviceName << std::endl;
    this->vk_physical_device_ = dev;
  }
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
  device_features.tessellationShader = VK_TRUE;
  device_features.geometryShader = VK_TRUE;
  device_features.fillModeNonSolid = VK_TRUE;
  device_features.shaderStorageImageExtendedFormats = VK_TRUE;

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
  uint32_t* comp_shader, *comp2_shader;
  size_t comp_shader_length, comp2_shader_length;

  if (this->shader_name_ == "volume") {
    comp_shader = (uint32_t*)src_shaders_shader_volume_comp_spv;
    comp_shader_length = src_shaders_shader_volume_comp_spv_len;
    comp2_shader = (uint32_t*)src_shaders_shader_2_volume_comp_spv;
    comp2_shader_length = src_shaders_shader_2_volume_comp_spv_len;
  }
  else if (this->shader_name_ == "minradial") {
    comp_shader = (uint32_t*)src_shaders_shader_minradial_comp_spv;
    comp_shader_length = src_shaders_shader_minradial_comp_spv_len;
    comp2_shader = (uint32_t*)src_shaders_shader_2_minradial_comp_spv;
    comp2_shader_length = src_shaders_shader_2_minradial_comp_spv_len;
  }
  else if (this->shader_name_ == "maxradial") {
    comp_shader = (uint32_t*)src_shaders_shader_maxradial_comp_spv;
    comp_shader_length = src_shaders_shader_maxradial_comp_spv_len;
    comp2_shader = (uint32_t*)src_shaders_shader_2_maxradial_comp_spv;
    comp2_shader_length = src_shaders_shader_2_maxradial_comp_spv_len;
  }
  else if (this->shader_name_ == "area") {
    comp_shader = (uint32_t*)src_shaders_shader_area_comp_spv;
    comp_shader_length = src_shaders_shader_area_comp_spv_len;
    comp2_shader = (uint32_t*)src_shaders_shader_2_area_comp_spv;
    comp2_shader_length = src_shaders_shader_2_area_comp_spv_len;
  }
  /*else if (this->shader_name_ == "direct_sunlight") {
    comp_shader = (uint32_t*)src_shaders_shader_direct_sunlight_comp_spv;
    comp_shader_length = src_shaders_shader_direct_sunlight_comp_spv_len;
    comp2_shader = (uint32_t*)src_shaders_shader_2_direct_sunlight_comp_spv;
    comp2_shader_length = src_shaders_shader_2_direct_sunlight_comp_spv_len;
  }*/
  else if (this->shader_name_ == "skyratio") {
    comp_shader = (uint32_t*)src_shaders_shader_skyratio_comp_spv;
    comp_shader_length = src_shaders_shader_skyratio_comp_spv_len;
    comp2_shader = (uint32_t*)src_shaders_shader_2_skyratio_comp_spv;
    comp2_shader_length = src_shaders_shader_2_skyratio_comp_spv_len;
  }

  // create vertex shader
  VkShaderModuleCreateInfo vertex_shader_info = {
    VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, // type (see documentation)
    nullptr, // next (see documentation, must be null)
    0, // flags (see documentation, must be 0)
    src_shaders_shader_vert_spv_len, // vertex shader size
    (uint32_t*)src_shaders_shader_vert_spv // vertex shader code
  };

  debug::handleVkResult(
    vkCreateShaderModule(
      this->vk_logical_device_, // the logical device
      &vertex_shader_info, // shader meta data
      nullptr, // allocation callback (see documentation)
      &this->vk_vertex_shader_ // the allocated memory for the logical device
    )
  );

  // tessellation control shader
  VkShaderModuleCreateInfo tessellation_control_shader_info = {
    VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, // type (see documentation)
    nullptr, // next (see documentation, must be null)
    0, // flags (see documentation, must be 0)
    src_shaders_shader_tesc_spv_len, // vertex shader size
    (uint32_t*)src_shaders_shader_tesc_spv // vertex shader code
  };

  debug::handleVkResult(
    vkCreateShaderModule(
      this->vk_logical_device_, // the logical device
      &tessellation_control_shader_info, // shader meta data
      nullptr, // allocation callback (see documentation)
      &this->vk_tessellation_control_shader_ // the allocated memory for the logical device
    )
  );

  // tessellation evaluation shader
  VkShaderModuleCreateInfo tessellation_evaluation_shader_info = {
    VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, // type (see documentation)
    nullptr, // next (see documentation, must be null)
    0, // flags (see documentation, must be 0)
    src_shaders_shader_tese_spv_len, // vertex shader size
    (uint32_t*)src_shaders_shader_tese_spv // vertex shader code
  };

  debug::handleVkResult(
    vkCreateShaderModule(
      this->vk_logical_device_, // the logical device
      &tessellation_evaluation_shader_info, // shader meta data
      nullptr, // allocation callback (see documentation)
      &this->vk_tessellation_evaluation_shader_ // the allocated memory for the logical device
    )
  );

  // create geoemtry shader
  VkShaderModuleCreateInfo geoemtry_shader_info = {
    VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, // type (see documentation)
    nullptr, // next (see documentation, must be null)
    0, // flags (see documentation, must be 0)
    src_shaders_shader_geom_spv_len, // geoemtry shader size
    (uint32_t*)src_shaders_shader_geom_spv // geoemtry shader code
  };

  debug::handleVkResult(
    vkCreateShaderModule(
      this->vk_logical_device_, // the logical device
      &geoemtry_shader_info, // shader meta data
      nullptr, // allocation callback (see documentation)
      &this->vk_geoemtry_shader_ // the allocated memory for the logical device
    )
  );

  // create fragment shader
  VkShaderModuleCreateInfo fragment_shader_info = {
    VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, // type (see documentation)
    nullptr, // next (see documentation, must be null)
    0, // flags (see documentation, must be 0)
    src_shaders_shader_frag_spv_len, // fragment shader size
    (uint32_t*)src_shaders_shader_frag_spv // fragment shader code
  };

  debug::handleVkResult(
    vkCreateShaderModule(
      this->vk_logical_device_, // the logical device
      &fragment_shader_info, // shader meta data
      nullptr, // allocation callback (see documentation)
      &this->vk_fragment_shader_ // the allocated memory for the logical device
    )
  );

  // create compute shader
  VkShaderModuleCreateInfo compute_shader_info = {
    VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, // type (see documentation)
    nullptr, // next (see documentation, must be null)
    0, // flags (see documentation, must be 0)
    comp_shader_length, // fragment shader size
    (uint32_t*)comp_shader // fragment shader code
  };

  debug::handleVkResult(
    vkCreateShaderModule(
      this->vk_logical_device_, // the logical device
      &compute_shader_info, // shader meta data
      nullptr, // allocation callback (see documentation)
      &this->vk_compute_shader_ // the allocated memory for the logical device
    )
  );

  // create compute shader
  VkShaderModuleCreateInfo compute_shader2_info = {
    VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, // type (see documentation)
    nullptr, // next (see documentation, must be null)
    0, // flags (see documentation, must be 0)
    comp2_shader_length, // fragment shader size
    (uint32_t*)comp2_shader // fragment shader code
  };

  debug::handleVkResult(
    vkCreateShaderModule(
      this->vk_logical_device_, // the logical device
      &compute_shader2_info, // shader meta data
      nullptr, // allocation callback (see documentation)
      &this->vk_compute_shader_2_ // the allocated memory for the logical device
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
    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, // initial layout
    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL // final layout
  };

  VkAttachmentDescription depth_attachment_description = {
    0, // flags (see documentation, 1 option)
    this->depth_stencil_format_, // color format
    VK_SAMPLE_COUNT_1_BIT, // num samples per fragment
    VK_ATTACHMENT_LOAD_OP_CLEAR, // operation when loading
    VK_ATTACHMENT_STORE_OP_STORE, // operation when storing
    VK_ATTACHMENT_LOAD_OP_DONT_CARE, // stencil operation when loading
    VK_ATTACHMENT_STORE_OP_DONT_CARE, // stencil operation when storing
    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, // initial layout
    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL // final layout
  };

  VkAttachmentDescription attachment_descriptions[] = {
    color_attachment_description,
    depth_attachment_description
  };


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
    2, // attachment count
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

void Context::InitializeVkDescriptorSetLayout() {
  // Graphics
  VkDescriptorSetLayoutBinding graphicsLayoutBinding = {};
  graphicsLayoutBinding.binding = 0;
  graphicsLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  graphicsLayoutBinding.descriptorCount = 1;
  graphicsLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_GEOMETRY_BIT;

  VkDescriptorSetLayoutCreateInfo graphicsLayoutInfo = {};
  graphicsLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  graphicsLayoutInfo.bindingCount = 1;
  graphicsLayoutInfo.pBindings = &graphicsLayoutBinding;

  debug::handleVkResult(
    vkCreateDescriptorSetLayout(
      this->vk_logical_device_,
      &graphicsLayoutInfo,
      nullptr,
      &this->vk_graphics_descriptor_set_layout_
    )
  );

  // Compute
  VkDescriptorSetLayoutBinding computeLayoutBindingIn = {};
  computeLayoutBindingIn.binding = 0;
  computeLayoutBindingIn.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
  computeLayoutBindingIn.descriptorCount = 1;
  computeLayoutBindingIn.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

  VkDescriptorSetLayoutBinding computeLayoutBindingOut = {};
  computeLayoutBindingOut.binding = 1;
  computeLayoutBindingOut.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  computeLayoutBindingOut.descriptorCount = 1;
  computeLayoutBindingOut.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

  VkDescriptorSetLayoutBinding computeLayoutBindingTmp = {};
  computeLayoutBindingTmp.binding = 2;
  computeLayoutBindingTmp.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  computeLayoutBindingTmp.descriptorCount = 1;
  computeLayoutBindingTmp.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;


  std::vector<VkDescriptorSetLayoutBinding> bindings = {
    computeLayoutBindingIn,
    computeLayoutBindingOut,
    computeLayoutBindingTmp
  };

  VkDescriptorSetLayoutCreateInfo computeLayoutInfo = {};
  computeLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  computeLayoutInfo.bindingCount = bindings.size();
  computeLayoutInfo.pBindings = bindings.data();

  debug::handleVkResult(
    vkCreateDescriptorSetLayout(
      this->vk_logical_device_,
      &computeLayoutInfo,
      nullptr,
      &this->vk_compute_descriptor_set_layout_
    )
  );
}

void Context::InitializeVkDescriptorPool() {
  // graphics
  VkDescriptorPoolSize graphicsPoolSize = {};
  graphicsPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  graphicsPoolSize.descriptorCount = 1;

  // compute
  VkDescriptorPoolSize computePoolSizeIn = {};
  computePoolSizeIn.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
  computePoolSizeIn.descriptorCount = 1;

  VkDescriptorPoolSize computePoolSizeTmp = {};
  computePoolSizeTmp.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  computePoolSizeTmp.descriptorCount = 2;


  // create pool
  std::vector<VkDescriptorPoolSize> poolSizes = {
    graphicsPoolSize,
    computePoolSizeIn,
    computePoolSizeTmp
  };

  VkDescriptorPoolCreateInfo poolInfo = {};
  poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.poolSizeCount = poolSizes.size();
  poolInfo.pPoolSizes = poolSizes.data();
  poolInfo.maxSets = 2;

  debug::handleVkResult(
    vkCreateDescriptorPool(this->vk_logical_device_, &poolInfo, nullptr, &this->vk_descriptor_pool_)
  );
}

void Context::InitializeVkGraphicsPipelineLayout() {
  VkPushConstantRange push_constant_range = {
     VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_GEOMETRY_BIT,
     0,
     sizeof(UniformBufferObject)
   };

   std::vector<VkPushConstantRange> push_constant_ranges = {
     push_constant_range
   };

  // Define Pipeline layout
  VkPipelineLayoutCreateInfo pipeline_layout_info = {
    VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, // sType
    nullptr, // next (see documentation, must be null)
    0, // flags (see documentation, must be 0)
    1, // layout count
    &this->vk_graphics_descriptor_set_layout_, // layouts
    (uint32_t)push_constant_ranges.size(), // push constant range count
    push_constant_ranges.data() // push constant ranges
  };

  // Create pipeline layout
  debug::handleVkResult(
    vkCreatePipelineLayout(
      this->vk_logical_device_,
      &pipeline_layout_info,
      nullptr,
      &this->vk_graphics_pipeline_layout_
    )
  );
}

void Context::InitializeVkComputePipelineLayout() {
  // Define Pipeline layout
  VkPipelineLayoutCreateInfo pipeline_layout_info = {
    VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, // sType
    nullptr, // next (see documentation, must be null)
    0, // flags (see documentation, must be 0)
    1, // layout count
    &this->vk_compute_descriptor_set_layout_, // layouts
    0, // push constant range count
    nullptr // push constant ranges
  };

  // Create pipeline layout
  debug::handleVkResult(
    vkCreatePipelineLayout(
      this->vk_logical_device_,
      &pipeline_layout_info,
      nullptr,
      &this->vk_compute_pipeline_layout_
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

  VkPipelineShaderStageCreateInfo tessellation_control_shader_stage_info = {
    VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, // sType (see documentation)
    nullptr, // next (see documentation, must be null)
    0, // flags (see documentation, must be 0)
    VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, // stage flag
    this->vk_tessellation_control_shader_, // shader module
    "main", // the pipeline's name
    nullptr // VkSpecializationInfo (see documentation)
  };

  VkPipelineShaderStageCreateInfo tessellation_evaluation_shader_stage_info = {
    VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, // sType (see documentation)
    nullptr, // next (see documentation, must be null)
    0, // flags (see documentation, must be 0)
    VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, // stage flag
    this->vk_tessellation_evaluation_shader_, // shader module
    "main", // the pipeline's name
    nullptr // VkSpecializationInfo (see documentation)
  };

  VkPipelineShaderStageCreateInfo geoemtry_shader_stage_info = {
    VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, // sType (see documentation)
    nullptr, // next (see documentation, must be null)
    0, // flags (see documentation, must be 0)
    VK_SHADER_STAGE_GEOMETRY_BIT, // stage flag
    this->vk_geoemtry_shader_, // shader module
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

  VkPipelineShaderStageCreateInfo shader_stages[] = {
    vertex_shader_stage_info,
    tessellation_control_shader_stage_info,
    tessellation_evaluation_shader_stage_info,
    geoemtry_shader_stage_info,
    fragment_shader_stage_info
  };

  // Get vertex data
  VkVertexInputBindingDescription vertex_binding = Vertex::getBindingDescription();
  std::vector<VkVertexInputAttributeDescription> vertex_attributes = Vertex::getAttributeDescriptions();

  // Define input loading
  // TODO: Load vertices correctly
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
    5, // number of stages (we have 2 shaders for now)
    shader_stages, // shader stage create infos
    &vertex_input_info, // vertex input info
    &input_assembly_info, // inpt assembly info
    &tessellation_info, // tesselation info
    &viewport_info, // viewport info
    &rasterizer_info, // rasterization info
    &multisampling_info, // multisampling info
    &depth_stencil_info, // depth stencil info
    &color_blend_info, // blending info
    nullptr, // dynamic states info (e.g. window size changes or so)
    this->vk_graphics_pipeline_layout_, // pipeline layout
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
      &this->vk_graphics_pipeline_ // allocated memory for the pipeline
    )
  );
}


void Context::InitializeVkComputePipeline() {
  // create pipeline shader stages
  VkPipelineShaderStageCreateInfo compute_shader_stage_info = {
    VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, // sType (see documentation)
    nullptr, // next (see documentation, must be null)
    0, // flags (see documentation, must be 0)
    VK_SHADER_STAGE_COMPUTE_BIT, // stage flag
    this->vk_compute_shader_, // shader module
    "main", // the pipeline's name
    nullptr // VkSpecializationInfo (see documentation)
  };

  VkPipelineShaderStageCreateInfo compute_shader_2_stage_info = {
    VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, // sType (see documentation)
    nullptr, // next (see documentation, must be null)
    0, // flags (see documentation, must be 0)
    VK_SHADER_STAGE_COMPUTE_BIT, // stage flag
    this->vk_compute_shader_2_, // shader module
    "main", // the pipeline's name
    nullptr // VkSpecializationInfo (see documentation)
  };

  // Define pipeline info
  VkComputePipelineCreateInfo pipeline_info = {
    VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO, // sType
    nullptr, // next (see documentation, must be null)
    0, // pipeline create flags (have no child pipelines, so don't care)
    compute_shader_stage_info, // shader stage create infos
    this->vk_compute_pipeline_layout_,
    VK_NULL_HANDLE,
    -1 // parent pipeline index
  };

  VkComputePipelineCreateInfo pipeline_info_2 = {
    VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO, // sType
    nullptr, // next (see documentation, must be null)
    0, // pipeline create flags (have no child pipelines, so don't care)
    compute_shader_2_stage_info, // shader stage create infos
    this->vk_compute_pipeline_layout_,
    VK_NULL_HANDLE,
    -1 // parent pipeline index
  };

  debug::handleVkResult(
    vkCreateComputePipelines(
      this->vk_logical_device_, // logical device
      VK_NULL_HANDLE, // pipeline cache // TODO: Add pipeline cache (?)
      1, // pipeline count
      &pipeline_info, // pipeline infos
      nullptr, // allocation callback
      &this->vk_compute_pipeline_ // allocated memory for the pipeline
    )
  );

  debug::handleVkResult(
    vkCreateComputePipelines(
      this->vk_logical_device_, // logical device
      VK_NULL_HANDLE, // pipeline cache // TODO: Add pipeline cache (?)
      1, // pipeline count
      &pipeline_info_2, // pipeline infos
      nullptr, // allocation callback
      &this->vk_compute_pipeline_2_ // allocated memory for the pipeline
    )
  );


}

void Context::InitializeVkMemory() {
  // vertex buffer
  this->CreateBuffer(
    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    sizeof(this->vertices_[0]) * this->vertices_.size(),
    &this->vk_vertex_buffer_, &this->vk_vertex_buffer_memory_);

  this->CreateBuffer(
    VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    sizeof(this->vertices_[0]) * this->vertices_.size(),
    &this->vk_vertex_staging_buffer_, &this->vk_vertex_staging_buffer_memory_);

  // index buffer
  this->CreateBuffer(
    VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    sizeof(this->indices_[0]) * this->indices_.size(),
    &this->vk_index_buffer_, &this->vk_index_buffer_memory_);

  this->CreateBuffer(
    VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    sizeof(this->indices_[0]) * this->indices_.size(),
    &this->vk_index_staging_buffer_, &this->vk_index_staging_buffer_memory_);

  // uniform buffer
  this->CreateBuffer(
    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    sizeof(UniformBufferObject),
    &this->vk_uniform_buffer_, &this->vk_uniform_buffer_memory_);

  this->CreateBuffer(
    VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    sizeof(UniformBufferObject),
    &this->vk_uniform_staging_buffer_, &this->vk_uniform_staging_buffer_memory_);

  // compute buffer
  this->CreateBuffer(
    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    sizeof(float)*this->workgroups[0],
    &this->vk_compute_tmp_buffer_, &this->vk_compute_tmp_buffer_memory_);

  this->CreateBuffer(
    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    this->compute_size_,
    &this->vk_compute_buffer_, &this->vk_compute_buffer_memory_);

  this->CreateBuffer(
    VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    this->compute_size_,
    &this->vk_compute_staging_buffer_, &this->vk_compute_staging_buffer_memory_);

  // color image
  this->CreateImage(this->color_format_,
    VK_IMAGE_LAYOUT_PREINITIALIZED,
    VK_IMAGE_TILING_OPTIMAL,
    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    &this->vk_color_image_,
    &this->vk_color_image_memory_);

  this->CreateImage(this->color_format_,
    VK_IMAGE_LAYOUT_PREINITIALIZED,
    VK_IMAGE_TILING_LINEAR,
    VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    &this->vk_color_staging_image_,
    &this->vk_color_staging_image_memory_);

  // depth image
  this->CreateImage(this->depth_stencil_format_,
    VK_IMAGE_LAYOUT_PREINITIALIZED,
    VK_IMAGE_TILING_OPTIMAL,
    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
    &this->vk_depth_stencil_image_,
    &this->vk_depth_stencil_image_memory_);

  this->CreateImage(this->depth_stencil_format_,
    VK_IMAGE_LAYOUT_PREINITIALIZED,
    VK_IMAGE_TILING_LINEAR,
    VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    &this->vk_depth_stencil_staging_image_,
    &this->vk_depth_stencil_staging_image_memory_);



  // image views
  this->CreateImageView(this->vk_depth_stencil_image_, this->depth_stencil_format_, VK_IMAGE_ASPECT_DEPTH_BIT, &this->vk_depth_stencil_imageview_);
  this->CreateImageView(this->vk_color_image_, this->color_format_, VK_IMAGE_ASPECT_COLOR_BIT, &this->vk_color_imageview_);

  // framebuffer
  this->CreateFrameBuffer();

  // graphics command buffers
  this->CreateCommandPool(&this->vk_graphics_command_pool_);
  this->CreateCommandBuffer(this->vk_graphics_command_pool_, &this->vk_graphics_commandbuffer_);

  // compute command buffers
  this->CreateCommandPool(&this->vk_compute_command_pool_);
  this->CreateCommandBuffer(this->vk_compute_command_pool_, &this->vk_compute_commandbuffer_);
  this->CreateCommandBuffer(this->vk_compute_command_pool_, &this->vk_compute_commandbuffer_2_);
}

// RENDERING

void Context::InitializeVkGraphicsCommandBuffers() {
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

  VkClearValue clear_values[] = {
    {0.0f, 0.0f, 0.0f, 1.0f},
    {1.0f, 0.0f}
  };

  VkRenderPassBeginInfo render_pass_info = {
    VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO, // sType
    nullptr, // pNext (see documentation, must be null)
    this->vk_render_pass_, // render pass
    this->vk_graphics_framebuffer_, // framebuffer
    {{0,0}, {this->render_width_, this->render_height_}}, // render area (VkRect2D)
    2, // number of clear values
    clear_values // clear values
  };

  vkCmdBeginRenderPass(
    this->vk_graphics_commandbuffer_, // command buffer
    &render_pass_info, // render pass info
    VK_SUBPASS_CONTENTS_INLINE // store contents in primary command buffer
  );

  vkCmdPushConstants(
     this->vk_graphics_commandbuffer_,
     this->vk_graphics_pipeline_layout_,
     VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_GEOMETRY_BIT,
     0,
     sizeof(UniformBufferObject),
     &this->uniform_
  );

  // bind graphics pipeline
  vkCmdBindPipeline(
    this->vk_graphics_commandbuffer_, // command buffer
    VK_PIPELINE_BIND_POINT_GRAPHICS, // pipeline type
    this->vk_graphics_pipeline_ // graphics pipeline
  );

  // vertex data
  VkBuffer vertexBuffers[] = {this->vk_vertex_buffer_};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(this->vk_graphics_commandbuffer_,
    0, // vertex buffer binding index
    1, // number of bindings
    vertexBuffers, // vertex buffers
    offsets // offsets
  );

  // uniform buffer object
  vkCmdBindDescriptorSets(
    this->vk_graphics_commandbuffer_,
    VK_PIPELINE_BIND_POINT_GRAPHICS,
    this->vk_graphics_pipeline_layout_,
    0,
    1,
    &this->vk_graphics_descriptor_set_,
    0,
    nullptr
  );

  vkCmdBindIndexBuffer(this->vk_graphics_commandbuffer_, this->vk_index_buffer_, 0, VK_INDEX_TYPE_UINT32);

  // draw
  vkCmdDrawIndexed(
    this->vk_graphics_commandbuffer_, // command buffer
    this->indices_.size(), // num indexes
    1, // num instances // TODO
    0, // first index
    0, // vertex index offset
    0 // first instance
  );

  vkCmdEndRenderPass(this->vk_graphics_commandbuffer_);

  debug::handleVkResult(
    vkEndCommandBuffer(
      this->vk_graphics_commandbuffer_
    )
  );
}

void Context::InitializeVkComputeCommandBuffers() {
  VkCommandBufferBeginInfo command_buffer_begin_info = {
    VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, // sType
    nullptr, // pNext (see documentation, must be null)
    VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT, // can be submitted and run simultaniously
    nullptr // VkCommandBufferInheritanceInfo (we don't need it)
  };

  debug::handleVkResult(
    vkBeginCommandBuffer(
      this->vk_compute_commandbuffer_,
      &command_buffer_begin_info
    )
  );

  vkCmdBindPipeline(
    this->vk_compute_commandbuffer_,
    VK_PIPELINE_BIND_POINT_COMPUTE,
    this->vk_compute_pipeline_
  );

  std::vector<VkDescriptorSet> descriptor_sets = {
    this->vk_compute_descriptor_set_
  };

  vkCmdBindDescriptorSets(
    this->vk_compute_commandbuffer_,
    VK_PIPELINE_BIND_POINT_COMPUTE,
    this->vk_compute_pipeline_layout_,
    0,
    descriptor_sets.size(),
    descriptor_sets.data(),
    0,
    0
  );

  vkCmdDispatch(
    this->vk_compute_commandbuffer_,
    this->workgroups[0],
    this->workgroups[1],
    this->workgroups[2]
  );

  debug::handleVkResult(
    vkEndCommandBuffer(
      this->vk_compute_commandbuffer_
    )
  );

  // next one
  VkCommandBufferBeginInfo command_buffer_begin_info2 = {
    VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, // sType
    nullptr, // pNext (see documentation, must be null)
    VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT, // can be submitted and run simultaniously
    nullptr // VkCommandBufferInheritanceInfo (we don't need it)
  };

  debug::handleVkResult(
    vkBeginCommandBuffer(
      this->vk_compute_commandbuffer_2_,
      &command_buffer_begin_info2
    )
  );

  vkCmdBindPipeline(
    this->vk_compute_commandbuffer_2_,
    VK_PIPELINE_BIND_POINT_COMPUTE,
    this->vk_compute_pipeline_2_
  );

  std::vector<VkDescriptorSet> descriptor_sets_2 = {
    this->vk_compute_descriptor_set_
  };

  vkCmdBindDescriptorSets(
    this->vk_compute_commandbuffer_2_,
    VK_PIPELINE_BIND_POINT_COMPUTE,
    this->vk_compute_pipeline_layout_,
    0,
    descriptor_sets_2.size(),
    descriptor_sets_2.data(),
    0,
    0
  );

  vkCmdDispatch(
    this->vk_compute_commandbuffer_2_,
    this->workgroups2[0],
    this->workgroups2[1],
    this->workgroups2[2]
  );

  debug::handleVkResult(
    vkEndCommandBuffer(
      this->vk_compute_commandbuffer_2_
    )
  );
}

void Context::InitializeVkImageLayouts() {
    this->TransformImageLayout(this->vk_depth_stencil_image_, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT);
    this->TransformImageLayout(this->vk_color_image_, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);
    this->TransformImageLayout(this->vk_color_staging_image_, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_ASPECT_COLOR_BIT);
}

void Context::VkDraw() {
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

void Context::VkCompute() {
  VkSubmitInfo submit_info = {
    VK_STRUCTURE_TYPE_SUBMIT_INFO, // sType,
    nullptr, // next (see documentaton, must be null)
    0, // wait semaphore count
    nullptr, // semaphore to wait for
    nullptr, // stage until next semaphore is triggered
    1, //
    &this->vk_compute_commandbuffer_,
    0,
    nullptr
  };

  vkResetFences(this->vk_logical_device_, 1, &this->vk_compute_fence_);
  debug::handleVkResult(
    vkQueueSubmit(
      this->vk_queue_compute_, // queue
      1, // num infos
      &submit_info, // info
      this->vk_compute_fence_// fence (we don't need it)
    )
  );

  vkQueueWaitIdle(this->vk_queue_compute_);

  VkSubmitInfo submit_info2 = {
    VK_STRUCTURE_TYPE_SUBMIT_INFO, // sType,
    nullptr, // next (see documentaton, must be null)
    0, // wait semaphore count
    nullptr, // semaphore to wait for
    nullptr, // stage until next semaphore is triggered
    1, //
    &this->vk_compute_commandbuffer_2_,
    0,
    nullptr
  };

  vkResetFences(this->vk_logical_device_, 1, &this->vk_compute_fence_);
  debug::handleVkResult(
    vkQueueSubmit(
      this->vk_queue_compute_, // queue
      1, // num infos
      &submit_info2, // info
      this->vk_compute_fence_// fence (we don't need it)
    )
  );
}

/// TRANSFER ROUTINES

void Context::SubmitVertexData() {
  // copy vertex data from host to staging buffer
  VkMemoryRequirements vertex_buffer_memory_requirements;
  vkGetBufferMemoryRequirements(
    this->vk_logical_device_,
    this->vk_vertex_buffer_,
    &vertex_buffer_memory_requirements
  );
  uint32_t buffersize = vertex_buffer_memory_requirements.size;

  void* vertex_data;
  vkMapMemory(this->vk_logical_device_, this->vk_vertex_staging_buffer_memory_, 0, buffersize, 0, &vertex_data);
  memcpy(vertex_data, this->vertices_.data(), (size_t)buffersize);
  vkUnmapMemory(this->vk_logical_device_, this->vk_vertex_staging_buffer_memory_);

  // copy from stating buffer to device local buffer
  VkCommandBuffer commandbuffer = this->BeginSingleTimeBuffer();

  VkBufferCopy copyRegion = {};
  copyRegion.srcOffset = 0; // Optional
  copyRegion.dstOffset = 0; // Optional
  copyRegion.size = buffersize;
  vkCmdCopyBuffer(commandbuffer, this->vk_vertex_staging_buffer_, this->vk_vertex_buffer_, 1, &copyRegion);

  this->EndSingleTimeBuffer(commandbuffer);
}

void Context::SubmitIndexData() {
  // copy vertex data from host to staging buffer
  VkMemoryRequirements buffer_memory_requirements;
  vkGetBufferMemoryRequirements(
    this->vk_logical_device_,
    this->vk_index_staging_buffer_,
    &buffer_memory_requirements
  );
  uint32_t buffersize = buffer_memory_requirements.size;

  void* vertex_data;
  vkMapMemory(this->vk_logical_device_, this->vk_index_staging_buffer_memory_, 0, buffersize, 0, &vertex_data);
  memcpy(vertex_data, this->indices_.data(), (size_t)buffersize);
  vkUnmapMemory(this->vk_logical_device_, this->vk_index_staging_buffer_memory_);

  // copy from stating buffer to device local buffer
  VkCommandBuffer commandbuffer = this->BeginSingleTimeBuffer();

  VkBufferCopy copyRegion = {};
  copyRegion.srcOffset = 0; // Optional
  copyRegion.dstOffset = 0; // Optional
  copyRegion.size = buffersize;
  vkCmdCopyBuffer(commandbuffer, this->vk_index_staging_buffer_, this->vk_index_buffer_, 1, &copyRegion);

  this->EndSingleTimeBuffer(commandbuffer);
}

void Context::SubmitUniformData() {
  // copy vertex data from host to staging buffer
  VkMemoryRequirements buffer_memory_requirements;
  vkGetBufferMemoryRequirements(
    this->vk_logical_device_,
    this->vk_uniform_staging_buffer_,
    &buffer_memory_requirements
  );
  uint32_t buffersize = buffer_memory_requirements.size;

  void* vertex_data;
  vkMapMemory(this->vk_logical_device_, this->vk_uniform_staging_buffer_memory_, 0, buffersize, 0, &vertex_data);
  memcpy(vertex_data, &this->uniform_, (size_t)buffersize);
  vkUnmapMemory(this->vk_logical_device_, this->vk_uniform_staging_buffer_memory_);

  // copy from stating buffer to device local buffer
  VkCommandBuffer commandbuffer = this->BeginSingleTimeBuffer();

  VkBufferCopy copyRegion = {};
  copyRegion.srcOffset = 0; // Optional
  copyRegion.dstOffset = 0; // Optional
  copyRegion.size = buffersize;
  vkCmdCopyBuffer(commandbuffer, this->vk_uniform_staging_buffer_, this->vk_uniform_buffer_, 1, &copyRegion);

  this->EndSingleTimeBuffer(commandbuffer);
}

void Context::RetrieveRenderImage(uint32_t i) {
  vkQueueWaitIdle(this->vk_queue_graphics_);
  vkWaitForFences(this->vk_logical_device_, 1, &this->vk_compute_fence_, VK_TRUE, UINT64_MAX);

  this->TransformImageLayout(this->vk_color_image_, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);
  this->TransformImageLayout(this->vk_color_staging_image_, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);
  this->CopyImage(this->vk_color_image_, this->vk_color_staging_image_, this->render_width_, this->render_height_, VK_IMAGE_ASPECT_COLOR_BIT);
  this->TransformImageLayout(this->vk_color_staging_image_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_ASPECT_COLOR_BIT);

  VkImageSubresource subresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0};
  VkSubresourceLayout subresource_layout;
  vkGetImageSubresourceLayout(this->vk_logical_device_, this->vk_color_staging_image_, &subresource, &subresource_layout);

  VkMemoryRequirements host_visible_memory_requirements;
  vkGetImageMemoryRequirements(
    this->vk_logical_device_,
    this->vk_color_staging_image_,
    &host_visible_memory_requirements
  );
  size_t image_size = host_visible_memory_requirements.size;
  void *data;
  void *pixels = malloc(image_size);
  vkMapMemory(this->vk_logical_device_, this->vk_color_staging_image_memory_, 0, image_size, 0, (void **)&data);
  memcpy(pixels, data, image_size);
  vkUnmapMemory(this->vk_logical_device_, this->vk_color_staging_image_memory_);
  uint8_t image[this->render_width_ * this->render_height_];
  for (uint32_t i = 0; i < 4 * this->render_width_ * this->render_height_; i += 4) {
    float px;
    memcpy(&px, (uint8_t*)pixels + i, 4);
    image[i/4] = floor(px*255);
  }
  //int stbi_write_png(char const *filename, int w, int h, int comp, const void *data, int stride_in_bytes);
  std::string filename = "debug_images/render/" + std::to_string(i) + ".png";
  stbi_write_png(filename.c_str(), this->render_width_, this->render_height_, 1, (void*)image, 0);
  free(pixels);
  this->TransformImageLayout(this->vk_color_image_, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);
}

void Context::RetrieveDepthImage(uint32_t i) {
  vkQueueWaitIdle(this->vk_queue_graphics_);
  vkWaitForFences(this->vk_logical_device_, 1, &this->vk_compute_fence_, VK_TRUE, UINT64_MAX);

  this->TransformImageLayout(this->vk_depth_stencil_image_, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT);
  this->TransformImageLayout(this->vk_depth_stencil_staging_image_, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_DEPTH_BIT);
  this->CopyImage(this->vk_depth_stencil_image_, this->vk_depth_stencil_staging_image_, this->render_width_, this->render_height_, VK_IMAGE_ASPECT_DEPTH_BIT);
  this->TransformImageLayout(this->vk_depth_stencil_staging_image_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_ASPECT_DEPTH_BIT);

  VkImageSubresource subresource = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 0};
  VkSubresourceLayout subresource_layout;
  vkGetImageSubresourceLayout(this->vk_logical_device_, this->vk_depth_stencil_staging_image_, &subresource, &subresource_layout);

  VkMemoryRequirements host_visible_memory_requirements;
  vkGetImageMemoryRequirements(
    this->vk_logical_device_,
    this->vk_depth_stencil_staging_image_,
    &host_visible_memory_requirements
  );
  size_t image_size = host_visible_memory_requirements.size;
  void *data;
  void *pixels = malloc(image_size);
  vkMapMemory(this->vk_logical_device_, this->vk_depth_stencil_staging_image_memory_, 0, image_size, 0, (void **)&data);
  memcpy(pixels, data, image_size);
  vkUnmapMemory(this->vk_logical_device_, this->vk_depth_stencil_staging_image_memory_);

  uint8_t image[this->render_width_ * this->render_height_];
  for (uint32_t i = 0; i < 4 * this->render_width_ * this->render_height_; i += 4) {
    float px;
    memcpy(&px, (uint8_t*)pixels + i, 4);
    image[i/4] = floor((px)*255);
  }

  //int stbi_write_png(char const *filename, int w, int h, int comp, const void *data, int stride_in_bytes);
  stbi_write_png(("debug_images/depth/" + std::to_string(i) + ".png").c_str(), this->render_width_, this->render_height_, 1, (void*)image, 0);
  free(pixels);
}

void Context::ResetResult() {
  // copy from stating buffer to device local buffer
  void *data;
  vkMapMemory(this->vk_logical_device_, this->vk_compute_staging_buffer_memory_, 0, this->compute_size_, 0, (void **)&data);
  memcpy(data, (void*)&this->compute_default_value_, this->compute_size_);
  vkUnmapMemory(this->vk_logical_device_, this->vk_compute_staging_buffer_memory_);

  VkCommandBuffer commandbuffer = this->BeginSingleTimeBuffer();
  VkBufferCopy copyRegion = {};
  copyRegion.srcOffset = 0; // Optional
  copyRegion.dstOffset = 0; // Optional
  copyRegion.size = this->compute_size_;
  vkCmdCopyBuffer(commandbuffer, this->vk_compute_staging_buffer_, this->vk_compute_buffer_, 1, &copyRegion);
  this->EndSingleTimeBuffer(commandbuffer);
}

void* Context::RetrieveResult() {
  // copy from stating buffer to device local buffer
  VkCommandBuffer commandbuffer = this->BeginSingleTimeBuffer();
  VkBufferCopy copyRegion = {};
  copyRegion.srcOffset = 0; // Optional
  copyRegion.dstOffset = 0; // Optional
  copyRegion.size = this->compute_size_;
  vkCmdCopyBuffer(commandbuffer, this->vk_compute_buffer_, this->vk_compute_staging_buffer_, 1, &copyRegion);
  this->EndSingleTimeBuffer(commandbuffer);

  void *data;
  void *result = malloc(this->compute_size_);
  vkMapMemory(this->vk_logical_device_, this->vk_compute_staging_buffer_memory_, 0, this->compute_size_, 0, (void **)&data);
  memcpy(result, data, this->compute_size_);
  vkUnmapMemory(this->vk_logical_device_, this->vk_compute_staging_buffer_memory_);

  return result;
}

void Context::RetrieveComputeImage(uint32_t i) {
  vkQueueWaitIdle(this->vk_queue_graphics_);
  vkWaitForFences(this->vk_logical_device_, 1, &this->vk_compute_fence_, VK_TRUE, UINT64_MAX);

  this->TransformImageLayout(this->vk_compute_image_, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);
  this->TransformImageLayout(this->vk_color_staging_image_, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT);
  this->CopyImage(this->vk_compute_image_, this->vk_color_staging_image_, this->render_width_, this->render_height_, VK_IMAGE_ASPECT_COLOR_BIT);
  this->TransformImageLayout(this->vk_color_staging_image_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_ASPECT_COLOR_BIT);

  VkImageSubresource subresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0};
  VkSubresourceLayout subresource_layout;
  vkGetImageSubresourceLayout(this->vk_logical_device_, this->vk_color_staging_image_, &subresource, &subresource_layout);

  VkMemoryRequirements host_visible_memory_requirements;
  vkGetImageMemoryRequirements(
    this->vk_logical_device_,
    this->vk_color_staging_image_,
    &host_visible_memory_requirements
  );
  size_t image_size = host_visible_memory_requirements.size;
  void *data;
  void *pixels = malloc(image_size);
  vkMapMemory(this->vk_logical_device_, this->vk_color_staging_image_memory_, 0, image_size, 0, (void **)&data);
  memcpy(pixels, data, image_size);
  vkUnmapMemory(this->vk_logical_device_, this->vk_color_staging_image_memory_);
/*
  uint8_t image[this->render_width_ * this->render_height_];
  for (uint32_t i = 0; i < 4 * this->render_width_ * this->render_height_; i += 4) {
    float px;
    memcpy(&px, (uint8_t*)pixels + i, 4);
    image[i/4] = floor(px*255);
  }
*/
  //int stbi_write_png(char const *filename, int w, int h, int comp, const void *data, int stride_in_bytes);
  stbi_write_png(("debug_images/compute" + std::to_string(i) + ".png").c_str(), this->render_width_, this->render_height_, 4, (void*)pixels, 0);
  free(pixels);
}


/// CREATION ROUTINES

void Context::CreateBuffer(VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryflags, uint32_t size, VkBuffer* buffer, VkDeviceMemory* buffer_memory) {
  VkBufferCreateInfo bufferInfo = {};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = size;
  bufferInfo.usage = usage;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  debug::handleVkResult(
    vkCreateBuffer(this->vk_logical_device_, &bufferInfo, nullptr, buffer)
  );

  VkMemoryRequirements memory_requirements;
  vkGetBufferMemoryRequirements(this->vk_logical_device_, *buffer, &memory_requirements);

  // find suitible memor-type
  uint32_t memory_type = 0;
  uint32_t memory_type_bits = memory_requirements.memoryTypeBits;
  VkPhysicalDeviceMemoryProperties physical_device_memory_properties;
  vkGetPhysicalDeviceMemoryProperties(this->vk_physical_device_, &physical_device_memory_properties);
  for (uint32_t i = 0; i < physical_device_memory_properties.memoryTypeCount; i++) {
    if ((memory_type_bits & (1 << i)) && (physical_device_memory_properties.memoryTypes[i].propertyFlags & memoryflags) == memoryflags) {
      memory_type = i;
      break;
    }
  }

  VkMemoryAllocateInfo allocation_info = {
    VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, // sType
    nullptr, // pNext (see documentation, must be null)
    memory_requirements.size, // the memory size
    memory_type // the memory type index
  };

  debug::handleVkResult(
    vkAllocateMemory(
      this->vk_logical_device_, // the logical devcie
      &allocation_info, // the allocation info
      nullptr, // allocation callback
      buffer_memory // allocated memory for memory object
    )
  );

  debug::handleVkResult(
    vkBindBufferMemory(
      this->vk_logical_device_, // the logical device
      *buffer, // the buffer
      *buffer_memory, // the buffer memory
      0 // the offset in the memory
    )
  );
}

void Context::CreateGraphicsDescriptorSet(VkDescriptorSetLayout layouts[], VkDescriptorSet* descriptor_set) {
  VkDescriptorSetAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = this->vk_descriptor_pool_;
  allocInfo.descriptorSetCount = 1;
  allocInfo.pSetLayouts = layouts;

  debug::handleVkResult(
    vkAllocateDescriptorSets(this->vk_logical_device_, &allocInfo, descriptor_set)
  );
}

void Context::UpdateGraphicsDescriptorSet(uint32_t size, VkBuffer buffer, VkDescriptorSet* descriptor_set) {
  VkDescriptorBufferInfo bufferInfo = {};
  bufferInfo.buffer = buffer;
  bufferInfo.offset = 0;
  bufferInfo.range = size;

  VkWriteDescriptorSet descriptorWrite = {};
  descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  descriptorWrite.dstSet = *descriptor_set;
  descriptorWrite.dstBinding = 0;
  descriptorWrite.dstArrayElement = 0;
  descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  descriptorWrite.descriptorCount = 1;
  descriptorWrite.pBufferInfo = &bufferInfo;
  descriptorWrite.pImageInfo = nullptr; // Optional
  descriptorWrite.pTexelBufferView = nullptr; // Optional

  vkUpdateDescriptorSets(this->vk_logical_device_, 1, &descriptorWrite, 0, nullptr);
}

void Context::CreateComputeDescriptorSets() {
  std::vector<VkDescriptorSet> descriptor_sets = {
    this->vk_compute_descriptor_set_,
  };

  VkDescriptorSetAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = this->vk_descriptor_pool_;
  allocInfo.descriptorSetCount = descriptor_sets.size();
  allocInfo.pSetLayouts = &this->vk_compute_descriptor_set_layout_;

  debug::handleVkResult(
    vkAllocateDescriptorSets(this->vk_logical_device_, &allocInfo, &this->vk_compute_descriptor_set_)
  );
}

void Context::UpdateComputeDescriptorSets() {
  VkDescriptorImageInfo image_in_info = { // TODO: COMPUTESHADERTODO - Change to bufferinfo
    VK_NULL_HANDLE,
    this->vk_color_imageview_,
    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
  };

  // Output: Image
  VkDescriptorBufferInfo buffer_out_info = {};
  buffer_out_info.buffer = this->vk_compute_buffer_;
  buffer_out_info.offset = 0;
  buffer_out_info.range = sizeof(float);

  VkDescriptorBufferInfo buffer_tmp_info = {};
  buffer_tmp_info.buffer = this->vk_compute_tmp_buffer_;
  buffer_tmp_info.offset = 0;
  buffer_tmp_info.range = sizeof(float)*this->workgroups[0];

  std::vector<VkDescriptorImageInfo> in_infos = {
    image_in_info
  };

  std::vector<VkDescriptorBufferInfo> out_infos = {
    buffer_out_info
  };

  std::vector<VkDescriptorBufferInfo> tmp_infos = {
    buffer_tmp_info
  };

  VkWriteDescriptorSet compute_in_descriptor_write = {};
  compute_in_descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  compute_in_descriptor_write.dstSet = this->vk_compute_descriptor_set_;
  compute_in_descriptor_write.dstBinding = 0;
  compute_in_descriptor_write.dstArrayElement = 0;
  compute_in_descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE; // TODO: COMPUTESHADERTODO - Change to bufferinfo
  compute_in_descriptor_write.descriptorCount = in_infos.size();
  compute_in_descriptor_write.pImageInfo = in_infos.data();
  compute_in_descriptor_write.pBufferInfo = nullptr;
  compute_in_descriptor_write.pTexelBufferView = nullptr;

  VkWriteDescriptorSet compute_out_descriptor_write = {};
  compute_out_descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  compute_out_descriptor_write.dstSet = this->vk_compute_descriptor_set_;
  compute_out_descriptor_write.dstBinding = 1;
  compute_out_descriptor_write.dstArrayElement = 0;
  compute_out_descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER; // TODO: COMPUTESHADERTODO - Change to bufferinfo
  compute_out_descriptor_write.descriptorCount = out_infos.size();
  compute_out_descriptor_write.pImageInfo = nullptr;
  compute_out_descriptor_write.pBufferInfo = out_infos.data();
  compute_out_descriptor_write.pTexelBufferView = nullptr;

  VkWriteDescriptorSet compute_tmp_descriptor_write = {};
  compute_tmp_descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  compute_tmp_descriptor_write.dstSet = this->vk_compute_descriptor_set_;
  compute_tmp_descriptor_write.dstBinding = 2;
  compute_tmp_descriptor_write.dstArrayElement = 0;
  compute_tmp_descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER; // TODO: COMPUTESHADERTODO - Change to bufferinfo
  compute_tmp_descriptor_write.descriptorCount = tmp_infos.size();
  compute_tmp_descriptor_write.pImageInfo = nullptr;
  compute_tmp_descriptor_write.pBufferInfo = tmp_infos.data();
  compute_tmp_descriptor_write.pTexelBufferView = nullptr;

  std::vector<VkWriteDescriptorSet> writedescriptor_sets = {
    compute_in_descriptor_write,
    compute_out_descriptor_write,
    compute_tmp_descriptor_write
  };

  vkUpdateDescriptorSets(this->vk_logical_device_, writedescriptor_sets.size(), writedescriptor_sets.data(), 0, nullptr);
}

void Context::CreateImage(VkFormat format, VkImageLayout layout, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags memoryflags, VkImage* image, VkDeviceMemory* image_memory) {
  VkImageCreateInfo image_info = {
    VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, // sType,
    nullptr, // pNext (see documentation, must be null)
    0, // image flags
    VK_IMAGE_TYPE_2D, // image type
    format, // image format
    {this->render_width_, this->render_height_, 1}, // image extent
    1, // level of detail = 1
    1, // layers = 1
    VK_SAMPLE_COUNT_1_BIT, // image sampling per pixel
    tiling, // linear tiling
    usage, // used for transfer
    VK_SHARING_MODE_EXCLUSIVE, // sharing between queue families
    1, // number queue families
    &this->queue_family_index_, // queue family index
    layout // initial layout
  };

  debug::handleVkResult(
    vkCreateImage(
      this->vk_logical_device_, // the logical device
      &image_info, // the image info
      nullptr, // allocation
      image// memory allocated for image object
    )
  );

  VkMemoryRequirements memory_requirements;
  vkGetImageMemoryRequirements(
    this->vk_logical_device_,
    *image,
    &memory_requirements
  );

  // get the first set bit in the type-bits. This bit is at the index position
  // of a supported memory type in the physical device
  uint32_t memory_type = 0;
  uint32_t memory_type_bits = memory_requirements.memoryTypeBits;
  VkPhysicalDeviceMemoryProperties physical_device_memory_properties;
    vkGetPhysicalDeviceMemoryProperties(this->vk_physical_device_, &physical_device_memory_properties);
  for (uint32_t i = 0; i < physical_device_memory_properties.memoryTypeCount; i++) {
    if ((memory_type_bits & (1 << i)) && (physical_device_memory_properties.memoryTypes[i].propertyFlags & memoryflags) == memoryflags) {
      memory_type = i;
      break;
    }
  }

  VkMemoryAllocateInfo allocation_info = {
    VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, // sType
    nullptr, // pNext (see documentation, must be null)
    memory_requirements.size, // the memory size
    memory_type // the memory type index
  };

  debug::handleVkResult(
    vkAllocateMemory(
      this->vk_logical_device_, // the logical devcie
      &allocation_info, // the allocation info
      nullptr, // allocation callback
      image_memory // allocated memory for memory object
    )
  );

  debug::handleVkResult(
    vkBindImageMemory(
      this->vk_logical_device_, // the logical device
      *image, // the image
      *image_memory, // the image memory
      0 // the offset in the memory
    )
  );
}

void Context::CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags flags, VkImageView* imageview) {
  VkImageViewCreateInfo imageview_info = {
    VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, // sType
    nullptr,// pNext (see documentation, must be null)
    0, // flags (see documentation, must be 0)
    image, // the image
    VK_IMAGE_VIEW_TYPE_2D, // the view type
    format, // the format
    {}, // stencil component mapping
    { // VkImageSubresourceRange (what's in the image)
      flags, // aspect flag
      0, // base level
      1, // level count
      0, // base layer
      1 // layer count
    }
  };

  debug::handleVkResult(
    vkCreateImageView(
      this->vk_logical_device_, // the logical device
      &imageview_info, // info
      nullptr, // allocation callback
      imageview // the allocated memory
    )
  );
}

void Context::CreateFrameBuffer() {
  // Create Framebuffers for color & stencil (color & depth)
  std::array<VkImageView, 2> attachments = {
    this->vk_color_imageview_,
    this->vk_depth_stencil_imageview_
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
}

void Context::CreateCommandPool(VkCommandPool* pool) {
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
      pool // the allocated memory
    )
  );
}

void Context::CreateCommandBuffer(VkCommandPool pool, VkCommandBuffer* buffer) {
  // create command buffers
  VkCommandBufferAllocateInfo command_buffer_info = {
    VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, // sType
    nullptr, // pNext (see documentation, must be null)
    pool, // the command pool
    VK_COMMAND_BUFFER_LEVEL_PRIMARY, // can be submitted to the queue
    1 // number of command buffers
  };

  debug::handleVkResult(
    vkAllocateCommandBuffers(
      this->vk_logical_device_, // the logical device
      &command_buffer_info, // info
      buffer // allocated memory
    )
  );
}

/// TRANSFORMATION ROUTINES

void Context::TransformImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageAspectFlags flags) {
    VkCommandBuffer commandBuffer = BeginSingleTimeBuffer();

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = flags;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    EndSingleTimeBuffer(commandBuffer);
}

void Context::CopyImage(VkImage srcImage, VkImage dstImage, uint32_t width, uint32_t height, VkImageAspectFlags aspectFlags) {
    VkCommandBuffer commandBuffer = BeginSingleTimeBuffer();

    VkImageSubresourceLayers subResource = {};
    subResource.aspectMask = aspectFlags;
    subResource.baseArrayLayer = 0;
    subResource.mipLevel = 0;
    subResource.layerCount = 1;

    VkImageCopy region = {};
    region.srcSubresource = subResource;
    region.dstSubresource = subResource;
    region.srcOffset = {0, 0, 0};
    region.dstOffset = {0, 0, 0};
    region.extent.width = this->render_width_;
    region.extent.height = this->render_height_;
    region.extent.depth = 1;

    vkCmdCopyImage(
        commandBuffer,
        srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1, &region
    );

    EndSingleTimeBuffer(commandBuffer);
}

/// UTILITY ROUTINES

VkCommandBuffer Context::BeginSingleTimeBuffer() {
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = this->vk_graphics_command_pool_;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(this->vk_logical_device_, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void Context::EndSingleTimeBuffer(VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(this->vk_queue_graphics_, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(this->vk_queue_graphics_);

    vkFreeCommandBuffers(this->vk_logical_device_, this->vk_graphics_command_pool_, 1, &commandBuffer);
}
