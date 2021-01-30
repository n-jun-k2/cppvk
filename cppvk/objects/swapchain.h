#pragma once

#include "../vk.h"
#include "object.h"
#include "logicaldevice.h"

namespace cppvk {

  /// <summary>
  /// 
  /// </summary>
  class LogicalDevice::Swapchain : public Object {
    public:
      using Object::Object;
      using pointer = std::shared_ptr<Swapchain>;
      using reference = std::weak_ptr<Swapchain>;
      class SwapchainBuilder;
    private:
      VkSwapchainKHR swapchain;
  };
  using Swapchain = LogicalDevice::Swapchain;

}