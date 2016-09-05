#include "quavis/context.h"

using namespace quavis;

Context::Context() {
  this->InitializeVkInstance();
  this->InitializeVkPhysicalDevice();
  this->InitializeVkLogicalDevice();
  this->InitializeVkPipeline();
  this->InitializeVkSwapChain();
}

Context::~Context() {
  // Implement context destructor
}

void Context::InitializeVkInstance() {

}

void Context::InitializeVkPhysicalDevice() {
  // TODO: Implement physical device initialization
}

void Context::InitializeVkLogicalDevice() {
  // TODO: Implement logical device initialization
}

void Context::InitializeVkPipeline() {
  // TODO: Implement pipeline initialization
}

void Context::InitializeVkSwapChain() {
  // TODO: Implement swap chain initialization
}
