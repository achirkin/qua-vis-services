#ifndef QUAVIS_VK_RESULT_H
#define QUAVIS_VK_RESULT_H

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
