#ifndef QUAVIS_DEBUG_H
#define QUAVIS_DEBUG_H

#include <vulkan/vulkan.h>
#include <iostream>

namespace quavis {
  namespace debug {
    // TODO: Add better debug handling
    extern bool handleVkResult(VkResult vkResult) {
      if (vkResult != 0) {
        std::cout << vkResult << std::endl;
        throw "An error occurred in the gpu initialization:";
      }
      return true;
    }
  }
}

#endif
