#ifndef QUAVIS_BUFFER_H
#define QUAVIS_BUFFER_H

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
    Buffer(LogicalDevice device, uint32_t size, VkBufferUsageFlags usage_flags, bool staging = true);

    /**
    * Destroys the buffer and all it's associated memory regions.
    */
    ~Buffer();

    /**
    * Writes data to the buffer. If staging is enabled, the transfer will be done
    * by choosing the first transfer queue of the logical device if no
    * other queue is specified.
    */
    void SetData(void* data, VkQueue queue = VK_NULL_HANDLE);

    /**
    * Retreives data from the buffer. If staging is enabled, the transfer will be done
    * by choosing the first transfer queue of the logical device if no
    * other queue is specified.
    */
    void* GetData(VkQueue queue = VK_NULL_HANDLE);

    /**
    * The VkBuffer object to be used in Vulkan methods
    */
    VkBuffer buffer;

  private:
    bool staging_ = false;

    VkMemory memory;
    VkMemory staging_memory;

    VkBuffer buffer;
    VkBuffer staging_buffer;

    LogicalDevice device_;
  };
}

#endif
