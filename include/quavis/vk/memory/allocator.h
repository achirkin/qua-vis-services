#ifndef QUAVIS_ALLOCATOR_H
#define QUAVIS_ALLOCATOR_H

namespace quavis {
  /**
  * The allocator class is used to manage the memory of a given physical device.
  */
  class Allocator {
  public:
    /**
    * Creates a new allocator object for the given physical device. The
    * allocator is responsible for selecting a given memory heap and allocating
    * the given memory.
    *
    * The class furthermore provides methods for sending to and retreiving from
    * buffers.
    */
    Allocator(LogicalDevice physical_device);

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

    /*
    * Writes data to a given memory object.
    */
    static void SetData(VkMemory destination_memory, void* data, size_t len);

    /*
    * Retreives data from a given memory object.
    */
    static void* GetData(VkMemory source_memory, size_t len);

  private:
    uint32_t GetHeap(VkMemoryPropertyFlags flags);
    
    LogicalDevice physical_device;
  };
}

#endif
