#pragma once

#include "../vk.h"
#include "../destroy.h"
#include "object.h"
#include "../physicaldevice/physicaldevice.h"

#include <memory>

namespace cppvk {
  class LogicalDevice : public cppvk::Object{
    public:
      using Object::Object;
      using pointer = std::shared_ptr<LogicalDevice>;
      using reference = std::weak_ptr<LogicalDevice>;
      class LogicalDeviceBuilder;
      class CommandPool;
    private:
      VkDevice device;
      std::unique_ptr<Destroy> destroy;
  };
}