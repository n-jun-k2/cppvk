#pragma once

#include "../vk.h"
#include "../context.h"
#include "builder.h"
#include "../physicaldeviceset.h"

#include "../objects/device.h"

#include <iostream>
#include <stdexcept>

namespace cppvk {

  /// <summary>
  /// Logical device builder pattern
  /// </summary>
  class DeviceBuilder : public cppvk::Builder {

  private:

    VkDeviceCreateInfo info;
    cppvk::PhysicalDeviceSet& physicaldevice;
		cppvk::QueueCreateInfos queueInfos;

    cppvk::Names layernames;
    cppvk::Names extensionnames;

    /// <summary>
    /// Creating an object instance
    /// </summary>
    /// <param name="arg"></param>
    /// <returns></returns>
    virtual cppvk::Device* createimpl(const VkAllocationCallbacks* arg) override {

      if (auto ctx = this->context.lock()) {


        info.queueCreateInfoCount = static_cast<uint32_t>(queueInfos.size());
        if (!queueInfos.empty())
          info.pQueueCreateInfos = queueInfos.data();

        physicaldevice.CreateDevice(&ctx->device, &info, arg);

        return new Device(ctx, [=](cppvk::Context& p) {
          std::cout << "vkDestroyDevice" << std::endl;
          vkDestroyDevice(p.device, arg);
          });

      }

      throw std::runtime_error("Vulkan context does not exist");
    }

  public:

    /// <summary>
    ///
    /// </summary>
    /// <param name="ctx"></param>
    /// <param name="dev"></param>
    explicit DeviceBuilder(cppvk::Context::Ptr ctx, cppvk::PhysicalDeviceSet& dev)
      : cppvk::Builder(ctx), physicaldevice(dev) {

      info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
      info.pNext = NULL;
      info.flags = 0;
      info.queueCreateInfoCount = 0;
      info.enabledLayerCount = 0;
      info.enabledExtensionCount = 0;

    }

    /// <summary>
    /// Create smart pointer
    /// </summary>
    /// <param name="callbacks"></param>
    /// <returns></returns>
    Device::Ptr create(const VkAllocationCallbacks* callbacks = VK_NULL_HANDLE) {
      return Device::Ptr(this->createimpl(callbacks));
    }

    ~DeviceBuilder() {
      for (auto queue : queueInfos) {
        delete[] queue.pQueuePriorities;
      }
    }
    DeviceBuilder()                                 = delete;
    DeviceBuilder(const DeviceBuilder&)             = default;
    DeviceBuilder& operator=(const DeviceBuilder&)  = default;
    DeviceBuilder(DeviceBuilder&&)                  = default;
    DeviceBuilder& operator=(DeviceBuilder&&)       = default;

		/// <summary>
		/// 
		/// </summary>
		/// <param name="layers"></param>
		/// <returns></returns>
		DeviceBuilder& layerNames(const Names& arg) {
			info.enabledLayerCount = static_cast<uint32_t>(arg.size());
			if(!arg.empty())info.ppEnabledLayerNames = arg.data();
			return *this;
		}

    /// <summary>
    /// 
    /// </summary>
    /// <param name="arg"></param>
    /// <returns></returns>
    DeviceBuilder& layerNames(Names&& arg) {
      layernames = std::move(arg);
      return layerNames(layernames);
    }

		/// <summary>
		/// 
		/// </summary>
		/// <param name="arg"></param>
		/// <returns></returns>
		DeviceBuilder& extensions(const Names& arg) {
			info.enabledExtensionCount = static_cast<uint32_t>(arg.size());
			if(!arg.empty())info.ppEnabledExtensionNames = arg.data();
			return *this;
		}

    /// <summary>
    /// 
    /// </summary>
    /// <param name="arg"></param>
    /// <returns></returns>
    DeviceBuilder& extensions(Names&& arg) {
      extensionnames = std::move(arg);
      return extensions(extensionnames);
    }

		/// <summary>
		/// 
		/// </summary>
		/// <param name="arg"></param>
		/// <returns></returns>
		DeviceBuilder& features(const VkPhysicalDeviceFeatures& arg) {
			info.pEnabledFeatures = &arg;
			return *this;
		}

    /// <summary>
    /// 
    /// </summary>
    /// <param name="arg"></param>
    /// <returns></returns>
    DeviceBuilder& features(VkPhysicalDeviceFeatures&& arg) {
      auto temp = std::move(arg);
      return features(temp);
    }

		/// <summary>
		/// 
		/// </summary>
		/// <param name="qinfo"></param>
		/// <returns></returns>
		DeviceBuilder& addQueueInfo(const VkDeviceQueueCreateInfo& qinfo) {
			queueInfos.push_back(qinfo);
			return *this;
		}

    /// <summary>
    /// 
    /// </summary>
    /// <param name="qinfo"></param>
    /// <returns></returns>
    DeviceBuilder& addQueueInfo(VkDeviceQueueCreateInfo&& qinfo) {
      queueInfos.push_back(std::move(qinfo));
      return *this;
    }

  };

}