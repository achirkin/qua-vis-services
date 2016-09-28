#ifndef QUAVIS_DESCRIPTORSET_H
#define QUAVIS_DESCRIPTORSET_H

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
    DescriptorSet(LogicalDevice device, DescriptorPool pool);

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
    uint32_t AddStorageImage(Image image);

    /**
    * Adds a storage buffer and returns its binding index.
    * The method should *never* be called after the Create() method has been
    * invoked.
    */
    uint32_t AddStorageBuffer(Buffer buffer);

    /**
    * Adds a uniform buffer and returns its binding index.
    * The method should *never* be called after the Create() method has been
    * invoked.
    */
    uint32_t AddUniformBuffer(Buffer buffer);

    /**
    * Creates the Vulkan object for this descriptor set
    */
    void Create();

    /**
    * Updates the descriptor set when one of the images, buffers or uniforms has
    * changed.
    */
    void Update(uint32_t index);

    VkDescriptorSet vk_handle;
  private:
    LogicalDevice device_;
  };
}

#endif
