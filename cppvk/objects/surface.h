#pragma once

#include "../vk.h"
#include "object.h"
#include "instance.h"

namespace cppvk {

  class Instance::Surface : public Object{
    public:
      using Object::Object;
      using pointer = std::shared_ptr<Surface>;
      using reference = std::weak_ptr<Surface>;
      class PhysicalDevice;
      class SurfaceBuilder;
      class PreSwapchainBuilder;
    private:
      VkSurfaceKHR surface;
  };

  using Surface = Instance::Surface;

}