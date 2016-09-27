#ifndef QUAVIS_MEMORY_H
#define QUAVIS_MEMORY_H

namespace quavis {
  /**
  * The allocator class is used to manage the memory of a given physical device.
  */
  class Allocator {
  public:
    /**
    * Creates a new allocator object for the given physical device. The
    * allocator is responsible for selecting a given memory heap and allocating
    * the given memory. Note that all memory created by this allocator
    * is destroyed upon it's destructor call.
    */
    Allocator(VkPhysicalDevice physical_device);

    /**
    * Destroys the allocator object and all memory that has been allocated by
    * it if it has not been freed yet.
    */
    ~Allocator();

    /**
    * Allocates a specified size of memory that supports the VkMemoryPropertyFlags
    * requirements such as VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT etc. If you
    * want to make sure that memory is allocated on the device, set the appropriate
    * flag. Otherwise it will often occur that memory is allocated on host-side
    */
    VkMemory Allocate(VkMemoryPropertyFlags flags, uint32_t size);

  private:
    uint32_t GetHeap(VkMemoryPropertyFlags flags);
    VkPhysicalDevice physical_device;
  };
}

#endif
