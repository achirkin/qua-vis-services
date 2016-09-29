#ifndef QUAVIS_BUFFER_H
#define QUAVIS_BUFFER_H

#include "quavis/vk/device/logicaldevice.h"
#include "quavis/vk/memory/allocator.h"
#include "quavis/vk/debug.h"

#include <vulkan/vulkan.h>

namespace quavis {
  /**
  * A wrapper around the VkBuffer struct.
  */
  class Buffer {
  public:
    /**
    * Creates a new buffer on the device. If staging is enabled, a staging buffer
    * will be created that is used for transfer between the host and the buffer
    * memory.
    *
    * If staging is enabled the buffer will be allocated using the following
    * flags:
    *  * VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    *
    * If staging disabled the memory will be initialized with
    *  * VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    *  * VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
    *  * VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    */
    Buffer(
      LogicalDevice* logical_device,
      Allocator* allocator,
      uint32_t size,
      VkBufferUsageFlags usage_flags,
      bool staging = true
    );

    /**
    * Destroys the buffer and all it's associated memory regions.
    */
    ~Buffer();

    /**
    * Writes data to the buffer.
    * Synchronization is responsibility of the caller.
    */
    void SetData(void* data, VkQueue queue);

    /**
    * Retreives data from the buffer.
    * Synchronization is responsibility of the caller.
    */
    void* GetData(VkQueue queue);

    /**
    * The VkBuffer object to be used in Vulkan methods
    */
    VkBuffer vk_handle;

    /**
    * The buffer size
    */
    uint32_t size;

  private:
    bool staging_ = false;

    VkDeviceMemory vk_memory_;
    VkDeviceMemory vk_staging_memory_;
    VkBuffer vk_staging_buffer_;

    LogicalDevice* logical_device_;
    Allocator* allocator_;

    const VkMemoryPropertyFlags staging_property_flags_ =
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
      VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
  };
}

#endif
