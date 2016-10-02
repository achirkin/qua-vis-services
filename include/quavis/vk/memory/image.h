#ifndef QUAVIS_IMAGE_H
#define QUAVIS_IMAGE_H

#include "quavis/vk/device/logicaldevice.h"
#include "quavis/vk/memory/allocator.h"
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
    Image(std::shared_ptr<LogicalDevice> logical_device, std::shared_ptr<Allocator> allocator, VkQueue queue, uint32_t width, uint32_t height, VkImageUsageFlags usage_flags, VkFormat format, VkImageLayout layout, VkImageAspectFlags aspect, bool staging = true);

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
    * Transforms the image layout.
    * Synchronization is responsibility of the caller.
    */
    void SetLayout(VkImage image, VkImageLayout old_layout, VkImageLayout layout, VkImageAspectFlags aspect_flags, VkQueue queue);

    /**
    * The VkImage object to be used in Vulkan methods
    */
    VkImage vk_handle;

    /**
    * The VkImageView object to be used in Vulkan methods
    */
    VkImageView vk_view;

    /**
    * The image's layout
    */
    VkImageLayout vk_layout = VK_IMAGE_LAYOUT_UNDEFINED;

    /**
     * The image's width
     */
    uint32_t width;

    /**
     * The image's height
     */
    uint32_t height;

    /**
     * THe image's format
     */
     VkFormat format;

  private:
    VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspect_flags);

    bool staging_ = false;
    uint32_t memory_size_;
    VkImageAspectFlags aspect_flags_;


    VkDeviceMemory vk_memory_;
    VkDeviceMemory vk_staging_memory_;
    VkImage vk_staging_image_;

    std::shared_ptr<LogicalDevice> logical_device_;
    std::shared_ptr<Allocator> allocator_;

    const VkMemoryPropertyFlags staging_property_flags_ =
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
      VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
  };
}

#endif
