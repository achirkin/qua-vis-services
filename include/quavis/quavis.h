#ifndef QUAVIS_QUAVIS_H
#define QUAVIS_QUAVIS_H

// quavis version
#include "quavis/version.h"
#include "quavis/context.h"

// vulkan base parts
#include "quavis/vk/result.h"
#include "quavis/vk/physicaldevice.h"
#include "quavis/vk/logicaldevice.h"
#include "quavis/vk/pipeline.h"
#include "quavis/vk/swapchain.h"

// debugging parts
#ifdef QUAVIS_DEBUG
  #include "quavis/vk/window.h"
#endif

#endif
