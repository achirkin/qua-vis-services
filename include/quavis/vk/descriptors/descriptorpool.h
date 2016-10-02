#ifndef QUAVIS_DESCRIPTORPOOL_H
#define QUAVIS_DESCRIPTORPOOL_H

#include "quavis/vk/device/logicaldevice.h"
#include "quavis/vk/debug.h"

#include <vulkan/vulkan.h>

namespace quavis {
  /**
  * A wrapper around the VkDescriptorPool object. It is currently limited on three types
  * of descriptors: images, buffers and uniforms.
  */
  class DescriptorPool {
  public:
    /**
    * Creates a new DescriptorPool for a maximum of num_sets DescriptorSets
    * consisting all together of the specified number of images, buffers and
    * uniforms.
    */
    DescriptorPool(std::shared_ptr<LogicalDevice> logical_device, uint32_t num_sets, uint32_t num_storage_images, uint32_t num_storage_buffers, uint32_t num_uniform_buffers);

    /**
    * Destroys the command pool safely. Note that all dependent objects need to
    * be destroyed beforehand.
    */
    ~DescriptorPool();

    /**
    * The corresponding vulkan object of the descriptor pool.
    */
    VkDescriptorPool vk_handle;

  private:
    std::shared_ptr<LogicalDevice> logical_device_;
  };
}

#endif
