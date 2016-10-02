#ifndef QUAVIS_DESCRIPTORSET_H
#define QUAVIS_DESCRIPTORSET_H

#include "quavis/vk/descriptors/descriptorpool.h"
#include "quavis/vk/device/logicaldevice.h"
#include "quavis/vk/memory/buffer.h"
#include "quavis/vk/memory/image.h"
#include "quavis/vk/debug.h"

#include <vulkan/vulkan.h>
#include <vector>

namespace quavis {

  /**
  * A wrapper around the VkDescriptorSet structure. A descriptor set is put
  * together stepwise. First, the descriptorset is initialized for a given
  * descriptor pool. Then, successively, different descriptors are added.
  *
  * Finally, call the methods Create() to initialize the descriptor set. Call
  * Update() to mark changes when one of the images, buffers or uniforms
  * has changed.
  */
  class DescriptorSet {
  public:
    /**
    * Creates a new DescriptorSet object for the given descriptor pool.
    */
    DescriptorSet(std::shared_ptr<LogicalDevice> device, std::shared_ptr<DescriptorPool> pool, uint32_t num_storage_images, uint32_t num_storage_buffers, uint32_t num_uniform_buffers);

    /**
    * Destroys the descriptor set. Note that all other dependent objects need
    * to be destroyed beforehand.
    */
    ~DescriptorSet();

    /**
    * Adds an image descriptor and returns the binding-index of the image.
    * The method should *never* be called after the Create() method has been
    * invoked.
    */
    uint32_t AddStorageImage(uint32_t index, std::shared_ptr<Image> image, VkShaderStageFlags shader_stages);

    /**
    * Adds a storage buffer and returns its binding index.
    * The method should *never* be called after the Create() method has been
    * invoked.
    */
    uint32_t AddStorageBuffer(uint32_t index, std::shared_ptr<Buffer> buffer, VkShaderStageFlags shader_stages);

    /**
    * Adds a uniform buffer and returns its binding index.
    * The method should *never* be called after the Create() method has been
    * invoked.
    */
    uint32_t AddUniformBuffer(uint32_t index, std::shared_ptr<Buffer> buffer, VkShaderStageFlags shader_stages);

    /**
    * Creates the Vulkan object for this descriptor set
    */
    void Create();

    /**
    * Updates the descriptor set when one of the images, buffers or uniforms has
    * changed.
    */
    void Update(uint32_t index);

    /**
    * The vulkan object associated with this descriptor set. VK_NULL_HANDLE
    * until Create() is called.
    */
    VkDescriptorSet vk_handle;

    /**
    * The vulkan layout associated with this descriptor set. VK_NULL_HANDLE
    * until Create() is called.
    */
    VkDescriptorSetLayout vk_layout;

  private:
    std::shared_ptr<LogicalDevice> logical_device_;
    std::shared_ptr<DescriptorPool> descriptor_pool_;

    std::vector<VkDescriptorSetLayoutBinding> layout_bindings_;
    std::vector<VkWriteDescriptorSet> write_sets_;
    std::vector<VkDescriptorBufferInfo> buffer_infos_;
    std::vector<VkDescriptorBufferInfo> uniform_infos_;
    std::vector<VkDescriptorImageInfo> image_infos_;
  };
}

#endif
