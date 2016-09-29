#include "quavis/vk/instance.h"
#include "quavis/vk/device/physicaldevice.h"
#include "quavis/vk/device/logicaldevice.h"

int main(int argc, char** argv) {
  quavis::Instance* instance = new quavis::Instance();
  quavis::PhysicalDevice* physicaldevice = new quavis::PhysicalDevice(instance);
  quavis::LogicalDevice* logicaldevice = new quavis::LogicalDevice(physicaldevice, 3);
}
