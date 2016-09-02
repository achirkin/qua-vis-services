#ifdef DEBUG
  #define GLFW_INCLUDE_VULKAN
  #include <GLFW/glfw3.h>
#else
  #include <vulkan/vulkan.h>
#endif

#include <stdexcept>
#include <memory>
#include <vector>

class VulkanTest {
public:

  VulkanTest() {
  }

  void windowLoop() {
    while (!glfwWindowShouldClose(this->window_)) {
        glfwPollEvents();
    }
  }

  void initializeVulkan() {
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    unsigned int glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;
    createInfo.enabledLayerCount = 0;

    if (vkCreateInstance(&createInfo, nullptr, &this->vk_instance_) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }

  }

  void initializeWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    this->window_ = glfwCreateWindow(1280, 1024, "Vulkan", nullptr, nullptr);
  }

  void initializePhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(this->vk_instance_, &deviceCount, nullptr);
    if (deviceCount == 0)
        throw std::runtime_error("failed to find GPUs with Vulkan support!");

    // pick a physical device
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(this->vk_instance_, &deviceCount, devices.data());
    for (const auto& device : devices) {
      // if everything is fine, we have found our device
      this->vk_physical_device_ = device;
      break;
    }

    // if no suitable device has been found
    if (this->vk_physical_device_ == VK_NULL_HANDLE)
        throw std::runtime_error("failed to find a suitable GPU!");
  }

  ~VulkanTest() {
    vkDestroyInstance(this->vk_instance_, nullptr);
  }

private:
  GLFWwindow* window_;
  VkInstance vk_instance_;
  VkPhysicalDevice vk_physical_device_ = VK_NULL_HANDLE;
};

int main(int argc, char** argv) {
  VulkanTest* ptr = new VulkanTest();
  ptr->initializeWindow();
  ptr->initializeVulkan();
  ptr->initializePhysicalDevice();
  ptr->windowLoop();
}
