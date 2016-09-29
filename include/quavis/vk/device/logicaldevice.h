#ifndef QUAVIS_LOGICALDEVICE_H
#define QUAVIS_LOGICALDEVICE_H

#include "quavis/vk/device/physicaldevice.h"
#include "quavis/vk/commands/commandpool.h"
#include "quavis/vk/debug.h"

#include <vulkan/vulkan.h>
#include <vector>
#include <algorithm>

namespace quavis {
  /**
  * The LogicalDevice class is a wrapper around the VkDevice struct. It
  * provides methods for easy queue initialization and management as well as
  * command pool initialization.
  *
  * The class is furthermore used to execute commands on the device. The general
  * workflow is:
  * 1. Choose queue from graphics_queues, compute_queues, transfer_queues
  * 2. buffer = device.BeginCommandBuffer(queue, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT)
  * 3. <do something with commandbuffer>
  * 4. device.EndCommandBuffer(buffer)
  * 5. device.SubmitCommandBuffer(queue, buffer)
  */
  class LogicalDevice {
  public:
    /**
    * Creates a new logical device on the selected physical device.
    * Optionally, the number of graphics- and compute queues can be
    * specfified.
    *
    * The method selects 3 (not necessarily different) queue families
    * from the specified device.
    * The graphics queues are guaranteed to be usable for graphics while
    * the compute queues are usable for compute shaders and
    * the transfer queues are usable for memory transfer.
    */
    LogicalDevice(PhysicalDevice* physical_device, uint32_t num_queues = 1);

    /**
    * Destroys the logical device safely. Note that all objects that are
    * dependant on the logical device need to be destroyed beforehand.
    */
    ~LogicalDevice();

    // TODO: Change tcommand buffer creation such that it's only dependent on queue_family_index_ / CommandPool
    /**
    * Begins a new command buffer. This function chooses the right command pool
    * for a given queue and delegates the command buffer creation
    * to the command pool object.
    */
    VkCommandBuffer BeginCommandBuffer(VkQueue queue,
      VkCommandBufferUsageFlags flags);

    /**
    * Ends and submits a command buffer. This function chooses the right command pool
    * for a given queue and delegates the command buffer ending
    * to the command pool object.
    */
    void EndCommandBuffer(VkCommandBuffer command_buffer);

    /**
    * Ends and submits a command buffer. This function chooses the right command pool
    * for a given queue and delegates the command buffer ending
    * to the command pool object.
    */
    void SubmitCommandBuffer(VkQueue queue, VkCommandBuffer command_buffer);

    /**
    * The vulkan handler to the logical device.
    */
    VkDevice vk_handle;

    /**
    * The graphics queues (used for render pass)
    */
    std::vector<VkQueue> queues;

  private:
    VkDeviceQueueCreateInfo GetQueueCreateInfos(uint32_t queue_family_index, uint32_t num);
    uint32_t GetQueueFamily(VkQueueFlags required_flags);
    VkQueue GetQueue(uint32_t queue_family_index, uint32_t queue_index);

    void CreateCommandPool(uint32_t queue_family_index);
    CommandPool GetCommandPool(uint32_t queue_family_index);

    PhysicalDevice* physical_device_;

    // Default features used for a logical device
    VkPhysicalDeviceFeatures features_;

    // Default extensions
    const std::vector<const char*> extensions_;
  };
}

#endif
