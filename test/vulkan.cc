#include "quavis/quavis.h"

#include <vulkan/vulkan.h>

#include <stdexcept>
#include <memory>
#include <vector>

class VulkanTest {
public:

  VulkanTest() {
  }

  void initializeVulkan() {
    this->context_ = new quavis::Context();
  }

  ~VulkanTest() {
    delete this->context_;
  }

private:
  quavis::Context* context_;
};

int main(int argc, char** argv) {
  VulkanTest* ptr = new VulkanTest();
  ptr->initializeVulkan();
  delete ptr;
}
