#pragma once

#include "../vk.h"
#include "logicaldevice.h"
#include "object.h"

namespace cppvk {
  class LogicalDevice::Image : public cppvk::Object {
    public:
      class ImageBuilder;

      using Object::Object;
      using pointer = std::shared_ptr<Image>;
      using reference = std::weak_ptr<Image>;

    private:
      VkImage image;
  };

  using Image = LogicalDevice::Image;
}