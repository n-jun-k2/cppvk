#pragma once

#include "../vk.h"
#include "../context.h"
#include "builder.h"

#include "../objects/device.h"
#include <stdexcept>

namespace cppvk {

  /// <summary>
  /// 
  /// </summary>
  class DeviceBuilder : public cppvk::Builder {

  private:

    VkDeviceCreateInfo info;

    /// <summary>
    /// 
    /// </summary>
    /// <param name="arg"></param>
    /// <returns></returns>
    virtual cppvk::Device* createimpl(const VkAllocationCallbacks* arg) override {

      if (auto ctx = this->context.lock()) {

      }

      throw std::runtime_error("Vulkan context does not exist");
    }

  public:

    /// <summary>
    /// 
    /// </summary>
    /// <param name="ctx"></param>
    explicit DeviceBuilder(cppvk::Context::Ptr ctx) : cppvk::Builder(ctx) {

    }

    /// <summary>
    /// 
    /// </summary>
    /// <param name="callbacks"></param>
    /// <returns></returns>
    Device::Ptr create(const VkAllocationCallbacks* callbacks = VK_NULL_HANDLE) {
      return Device::Ptr(this->createimpl(callbacks));
    }

    ~DeviceBuilder() = default;
    DeviceBuilder() = delete;
    DeviceBuilder(const DeviceBuilder&) = default;
    DeviceBuilder& operator=(const DeviceBuilder&) = default;
    DeviceBuilder(DeviceBuilder&&) = default;
    DeviceBuilder& operator=(DeviceBuilder&&) = default;



  };

}