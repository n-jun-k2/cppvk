#pragma once

#include "../vk.h"
#include "object.h"
#include "../physicaldevice/physicaldevice.h"

namespace cppvk {
  class PhysicalDevice::LogicalDevice : public cppvk::Object{
    public:
      using Object::Object;
      using pointer = std::shared_ptr<LogicalDevice>;
      using reference = std::weak_ptr<LogicalDevice>;
      class LogicalDeviceBuilder;
    private:
      VkDevice device;
  };
  using LogicalDevice = PhysicalDevice::LogicalDevice;
}