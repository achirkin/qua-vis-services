#ifndef QUAVIS_IMAGE_H
#define QUAVIS_IMAGE_H

namespace quavis {
  /**
  * A wrapper around the VkBuffer struct.
  */
  class Image {
  public:
    /**
    * Creates a new buffer on the device. If staging is enabled, a staging buffer
    * will be created that is used for transfer between the host and the buffer
    * memory.
    *
    * Upon initialization, the image will be initialized using the provided
    * format, tiling, layout and is made usable to the given queues.
    * Since image initialization requires to execute a command, see the documentation
    * of Image.Setlayout for details on the initial layout transformation.
    *
    * The queues parameter specifies the queues for which the image is made
    * available.
    *
    * If staging is enabled the image memory will be allocated using the
    * flags:
    *  * VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    *
    * If staging disabled the memory will be initialized with
    *  * VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    *  * VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
    *  * VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    */
    Image(LogicalDevice device, uint32_t width, uint32_t height, VkImageUsageFlags usage_flags, std::vector<uint32_t> queues, VkFormat format, VkImageLayout layout, VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL, bool staging = true);

    /**
    * Destroys the buffer and all it's associated memory regions.
    */
    ~Image();

    /**
    * Transforms the image layout. If staging is enabled, the transfer will be done
    * by choosing the first transfer queue of the logical device if no
    * other queue is specified.
    */
    void SetLayout(VkImageLayout layout, VkImageAspectFlags aspect_flags);

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
    VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;

    bool staging = false;

    VkMemory memory;
    VkMemory staging_memory;

    VkBuffer buffer;
    VkBuffer staging_buffer;

    LogicalDevice device_;
  };
}

#endif
