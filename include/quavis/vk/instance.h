#ifndef QUAVIS_INSTANCE_H
#define QUAVIS_INSTANCE_H

#include "quavis/version.h"
#include "quavis/vk/debug.h"

#include <vulkan/vulkan.h>
#include <vector>

namespace quavis {
  /**
  * A wrapper around the VkInstance structure.
  */
  class Instance {
  public:
    /**
    * Creates a new Vulkan instance and initializes all objects.
    */
    Instance();

    /**
    * Destroys the instance. Note that all dependent objects need to be
    * destroyed beforehand.
    */
    ~Instance();

    /**
    * The handle to the given instance.
    */
    VkInstance vk_handle;

  private:
    const std::vector<const char*> extensions_ = {
      "VK_EXT_debug_report"
    };

    const std::vector<const char*> layers_ = {
      "VK_LAYER_LUNARG_standard_validation"
    };
  };
}

#endif
