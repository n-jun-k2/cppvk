#pragma once

#include "../vk.h"
#include "object.h"
#include "instance.h"

namespace cppvk {
  class Instance::DebugUtilsMessenger : public cppvk::Object {
  public:
    using Object::Object;
    using pointer = std::shared_ptr<DebugUtilsMessenger>;
    using reference = std::weak_ptr<DebugUtilsMessenger>;
    class DebugUtilsMessengerBuilder;
  private:
    VkDebugUtilsMessengerEXT messenger;
  };
  using DebugUtilsMessenger = Instance::DebugUtilsMessenger;
}