#include "qua-vis/initializer.h"

#include <string>
#include <iostream>
#include <sstream>

class Context {
public:
  vk::Instance instance;
  std::vector<vk::PhysicalDevice> physicalDevices;
  vk::PhysicalDevice physicalDevice;
  vk::PhysicalDeviceProperties deviceProperties;
  vk::PhysicalDeviceFeatures deviceFeatures;
  vk::PhysicalDeviceMemoryProperties deviceMemoryProperties;

  void createContext() {
    {
      // Vulkan instance
      vk::ApplicationInfo appInfo;
      appInfo.pApplicationName = "VulkanExamples";
      appInfo.pEngineName = "VulkanExamples";
      appInfo.apiVersion = VK_API_VERSION_1_0;

      std::vector<const char*> enabledExtensions = { VK_KHR_SURFACE_EXTENSION_NAME };

      vk::InstanceCreateInfo instanceCreateInfo;
      instanceCreateInfo.pApplicationInfo = &appInfo;
      if (enabledExtensions.size() > 0) {
          instanceCreateInfo.enabledExtensionCount = (uint32_t)enabledExtensions.size();
          instanceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();
      }
      instance = vk::createInstance(instanceCreateInfo);
    }
    physicalDevices = instance.enumeratePhysicalDevices();
    physicalDevice = physicalDevices[0];
    deviceProperties = physicalDevice.getProperties();
    deviceFeatures = physicalDevice.getFeatures();
    deviceMemoryProperties = physicalDevice.getMemoryProperties();
  }

  void destroyContext() {
    instance.destroy();
  }
};

std::string toHumanSize(size_t size) {
    static const std::vector<std::string> SUFFIXES{ { "B", "KB", "MB", "GB", "TB", "PB"} };
    size_t suffixIndex = 0;
    while (suffixIndex < SUFFIXES.size() - 1 && size > 1024) {
        size >>= 10;
        ++suffixIndex;
    }

    std::stringstream buffer;
    buffer << size << " " << SUFFIXES[suffixIndex];
    return buffer.str();
}

void Initializer::Load() {
  Context context;
  context.createContext();
  std::cout << "Vulkan Context Created" << std::endl;
  std::cout << "Device Name:    " << context.deviceProperties.deviceName << std::endl;
  std::cout << "Device Type:    " << vk::to_string(context.deviceProperties.deviceType) << std::endl;

  std::cout << "Memory Heaps:  " << context.deviceMemoryProperties.memoryHeapCount << std::endl;
  for (size_t i = 0; i < context.deviceMemoryProperties.memoryHeapCount; ++i) {
      const auto& heap = context.deviceMemoryProperties.memoryHeaps[i];
      std::cout << "\tHeap " << i << " flags " << vk::to_string(heap.flags) << " size " << toHumanSize(heap.size) << std::endl;
  }
  std::cout << std::endl;
  std::cout << "Memory Types:  " << context.deviceMemoryProperties.memoryTypeCount << std::endl;
  for (size_t i = 0; i < context.deviceMemoryProperties.memoryTypeCount; ++i) {
      const auto type = context.deviceMemoryProperties.memoryTypes[i];
      std::cout << "\tType " << i << " flags " << vk::to_string(type.propertyFlags) << " heap " << type.heapIndex << std::endl;
  }
  std::cout << std::endl;
  std::cout << "Queues:" << std::endl;
  std::vector<vk::QueueFamilyProperties> queueProps = context.physicalDevice.getQueueFamilyProperties();

  for (size_t i = 0; i < queueProps.size(); ++i) {
      const auto& queueFamilyProperties = queueProps[i];
      std::cout << std::endl;
      std::cout << "Queue Family: " << i << std::endl;
      std::cout << "\tQueue Family Flags: " << vk::to_string(queueFamilyProperties.queueFlags) << std::endl;
      std::cout << "\tQueue Count: " << queueFamilyProperties.queueCount << std::endl;
  }

  uint32_t extensionCount = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
  std::vector<VkExtensionProperties> extensions(extensionCount);
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
  std::cout << "\nAvailable extensions:" << std::endl;
  for (const auto& extension : extensions) {
      std::cout << "\t" << extension.extensionName << std::endl;
  }

  std::cout << "Press enter to exit";
  std::cin.get();
  context.destroyContext();
}

int main(int argc, char** argv) {
  Initializer* init = new Initializer();
  init->Load();
}
