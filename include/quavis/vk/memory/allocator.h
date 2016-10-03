#ifndef QUAVIS_ALLOCATOR_H
#define QUAVIS_ALLOCATOR_H

#include "quavis/vk/device/logicaldevice.h"
#include "quavis/vk/debug.h"

#include <vulkan/vulkan.h>
#include <vector>

#include <string.h>

namespace quavis {
  /**
  * The allocator class is used to manage the memory of a given physical device.
  * It furthermore frees all allocated memory upon destruction.
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
    Allocator(std::shared_ptr<LogicalDevice> logical_device);

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
    VkDeviceMemory Allocate(VkMemoryRequirements memory_requirements, VkMemoryPropertyFlags flags);

    /*
    * Writes data to a given memory object.
    */
    void SetData(VkDeviceMemory destination_memory, void* data, uint32_t size);

    /*
    * Retreives data from a given memory object.
    */
    void* GetData(VkDeviceMemory source_memory, uint32_t size);

  private:
    uint32_t GetHeap(VkMemoryPropertyFlags flags);

    std::shared_ptr<LogicalDevice> logical_device_;

    std::vector<VkDeviceMemory> allocated_memory_;
  };
}

#endif
