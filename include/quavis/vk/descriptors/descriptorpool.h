#ifndef QUAVIS_DESCRIPTORPOOL_H
#define QUAVIS_DESCRIPTORPOOL_H

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
    DescriptorPool(LogicalDevice logical_device, uint32_t num_sets, uint32_t num_storage_images, uint32_t num_storage_buffers, uint32_t num_uniform_buffers);

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
    LogicalDevice device_;
  };
}

#endif
