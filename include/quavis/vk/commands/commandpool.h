#ifndef QUAVIS_COMMANDPOOL_H
#define QUAVIS_COMMANDPOOL_H

#include "quavis/vk/device/logicaldevice.h"
#include "quavis/vk/debug.h"

#include <vulkan/vulkan.h>

namespace quavis {
  // forward declaration of logical device
  class LogicalDevice;

  /**
  * A wrapper around VkCommandPool providing convenience methods for beginning
  * and ending a command buffer.
  */
  class CommandPool {
  public:
    /**
    * Creates a new command pool for the given queue family
    */
    CommandPool(LogicalDevice* device, uint32_t queue_family_index);

    /**
    * Safely destroys the VkCommandPool object. All dependents need to be
    * destroyed beforehand.
    */
    ~CommandPool();

    // TODO: Maybe add num_command_buffers argument
    /**
    * Begins a new command buffer for the given command pool.
    */
    VkCommandBuffer BeginCommandBuffer(VkCommandBufferUsageFlags flags);

    /**
    * Ends a command buffer. The submission of the command buffer is the
    * responsibility of the caller.
    */
    void EndCommandBuffer(VkCommandBuffer command_buffer);

    /**
    * The command pool object used in vulkan methods.
    */
    VkCommandPool vk_handle;

  private:
    LogicalDevice* device_;
    uint32_t queue_family_index_;
  };
}

#endif
