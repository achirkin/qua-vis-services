#ifndef QUAVIS_IMAGE_H
#define QUAVIS_IMAGE_H

#include "quavis/vk/device/logicaldevice.h"
#include "quavis/vk/debug.h"

#include <vulkan/vulkan.h>
#include <vector>

namespace quavis {
  /**
  * A wrapper around the VkBuffer struct.
  */
  class Image {
  public:
    /**
    * Creates a new image including image viewon the device.
    * If staging is enabled, a staging image will be created that is used for
    * transfer between the host and the image memory.
    *
    * Upon initialization, the image will be transformed using the provided
    * format and layout and is made usable to the given queues.
    * Since image initialization requires to execute a command, see the documentation
    * of Setlayout() for details on the initial layout transformation.
    *
    * By default, the tiling will be set to VK_IMAGE_TILING_OPTIMAL.
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
    Image(LogicalDevice device, uint32_t width, uint32_t height, VkImageUsageFlags usage_flags, std::vector<VkQueue> queues, VkFormat format, VkImageLayout layout, VkImageAspectFlags aspect, bool staging = true);

    /**
    * Destroys the buffer and all it's associated memory regions.
    */
    ~Image();

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
    * Copies the image to the specified image.
    * Synchronization is responsibility of the caller.
    */
    void Copy(Image destination, VkQueue queue);

    /**
    * Transforms the image layout.
    * Synchronization is responsibility of the caller.
    */
    void SetLayout(VkImageLayout layout, VkImageAspectFlags aspect_flags, VkQueue queue);

    /**
    * The VkImage object to be used in Vulkan methods
    */
    VkImage vk_handle;

    /**
    * The VkImageView object to be used in Vulkan methods
    */
    VkImageView vk_view;

  private:
    bool staging_ = false;

    VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
    std::vector<VkQueue> queues_;

    VkMemory vk_memory;
    VkMemory vk_staging_memory;
    VkBuffer vk_staging_buffer;

    LogicalDevice logical_device_;
  };
}

#endif
