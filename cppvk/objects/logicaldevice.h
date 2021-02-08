#pragma once

#include "../vk.h"
#include "../destroy.h"
#include "object.h"
#include "../physicaldevice/physicaldevice.h"

#include "../allocator/devicememory.h"

#include <memory>

namespace cppvk {
  class LogicalDevice : public cppvk::Object{
    public:
      using Object::Object;
      using pointer = std::shared_ptr<LogicalDevice>;
      using reference = std::weak_ptr<LogicalDevice>;
      using AllocateInfo = cppvk::DeviceMemoryAllocate<LogicalDevice>;
      class LogicalDeviceBuilder;
      class CommandPool;
      class Swapchain;
      class Image;
    private:
      VkDevice device;
      std::unique_ptr<Destroy> destroy;
  };
}