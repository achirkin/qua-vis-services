#ifndef QUAVIS_INSTANCE_H
#define QUAVIS_INSTANCE_H

#include "quavis/vk/debug.h"

#include <vulkan/vulkan.h>

namespace Quavis {
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
  };
}

#endif
