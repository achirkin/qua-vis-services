#ifndef QUAVIS_DEBUG_H
#define QUAVIS_DEBUG_H

#include <vulkan/vulkan.h>
#include <iostream>

namespace quavis {
  namespace vk {
    extern bool handleVkResult(VkResult vkResult) {
      std::cout << vkResult << std::endl;
      if (vkResult < 0)
        throw "An error occurred in the gpu initialization";
      // TODO: add better error handling
      return true;
    }
  }
}

#endif
