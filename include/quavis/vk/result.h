#ifndef QUAVIS_VK_RESULT_H
#define QUAVIS_VK_RESULT_H

#include <vulkan/vulkan.h>

namespace quavis {
  namespace vk {
    namespace {
      bool handleVkResult(VkResult vkResult) {
        if (vkResult < 0)
          throw "An error occurred in the gpu initialization";
        // TODO: add better error handling

        /*
        switch (vkResult) {
          case VK_SUCCESS:
          case VK_NOT_READY:
          case VK_TIMEOUT:
          case VK_EVENT_SET:
          case VK_EVENT_RESET:
          case VK_INCOMPLETE:
            return true;
            // TODO: Handle vulkan success results
          break;

          case VK_ERROR_OUT_OF_HOST_MEMORY:
          case VK_ERROR_OUT_OF_DEVICE_MEMORY:
          case VK_ERROR_INITIALIZATION_FAILED:
          case VK_ERROR_DEVICE_LOST:
          case VK_ERROR_MEMORY_MAP_FAILED:
          case VK_ERROR_LAYER_NOT_PRESENT:
          case VK_ERROR_EXTENSION_NOT_PRESENT:
          case VK_ERROR_FEATURE_NOT_PRESENT:
          case VK_ERROR_INCOMPATIBLE_DRIVER:
          case VK_ERROR_TOO_MANY_OBJECTS:
          case VK_ERROR_FORMAT_NOT_SUPPORTED:
            return false;
            // TODO: Handle vulkan error results
          break;
        }
        */
      }
    }
  }
}

#endif
