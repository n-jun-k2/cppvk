#pragma once

#include "../vk.h"
#include "../builders\Ibuilder.h"
#include "../physicaldevice/physicaldevice.h"
#include "../objects/logicaldevice.h"
#include <iostream>
#include <stdexcept>

namespace cppvk {

  /// <summary>
  /// Logical device builder pattern
  /// </summary>
  class LogicalDevice::LogicalDeviceBuilder : public IBuilder {

  private:

    VkDeviceCreateInfo info;
		cppvk::QueueCreateInfos queueInfos;
    cppvk::Instance::reference refInstance;
    cppvk::PhysicalDevice::reference refPhysicalDevice;

    cppvk::Names layernames;
    cppvk::Names extensionnames;

    /// <summary>
    /// Creating an object instance
    /// </summary>
    /// <param name="arg"></param>
    /// <returns></returns>
    virtual cppvk::LogicalDevice* createimpl(const VkAllocationCallbacks* arg) override {
      auto pPhysicalDevie = refPhysicalDevice.lock();
      auto pInstance = refInstance.lock();
      if (pPhysicalDevie && pInstance) {

        auto pLogicalDevice = new LogicalDevice(pInstance);
        auto& vkDevice = pLogicalDevice->device;
        auto& vkPhysicalDevice = pPhysicalDevie->physicalDevice;

        info.queueCreateInfoCount = static_cast<uint32_t>(queueInfos.size());
        if (!queueInfos.empty())
          info.pQueueCreateInfos = queueInfos.data();

        checkVk(vkCreateDevice(vkPhysicalDevice, &info, arg, &vkDevice));

         *(pInstance->destroy) += [=]() {
          std::cout << "vkDestroyDevice" << std::endl;
          vkDestroyDevice(vkDevice, arg);
        };

         return pLogicalDevice;

      }

      throw std::runtime_error("Vulkan context does not exist");
    }

  public:

    /// <summary>
    ///
    /// </summary>
    /// <param name="ctx"></param>
    /// <param name="dev"></param>
    explicit LogicalDeviceBuilder(cppvk::Instance::reference _instance, cppvk::PhysicalDevice::reference _physicaldevice) :
    refInstance(_instance), refPhysicalDevice(_physicaldevice) {
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
    cppvk::LogicalDevice::pointer create(const VkAllocationCallbacks* callbacks = VK_NULL_HANDLE) {
      return LogicalDevice::pointer(this->createimpl(callbacks));
    }

    ~LogicalDeviceBuilder() {
      for (auto queue : queueInfos) {
        delete[] queue.pQueuePriorities;
      }
    }
    LogicalDeviceBuilder()                                 = delete;
    LogicalDeviceBuilder(const LogicalDeviceBuilder&)             = default;
    LogicalDeviceBuilder& operator=(const LogicalDeviceBuilder&)  = default;
    LogicalDeviceBuilder(LogicalDeviceBuilder&&)                  = default;
    LogicalDeviceBuilder& operator=(LogicalDeviceBuilder&&)       = default;

		/// <summary>
		///
		/// </summary>
		/// <param name="layers"></param>
		/// <returns></returns>
		LogicalDeviceBuilder& layerNames(const Names& arg) {
			info.enabledLayerCount = static_cast<uint32_t>(arg.size());
			if(!arg.empty())info.ppEnabledLayerNames = arg.data();
			return *this;
		}

    /// <summary>
    ///
    /// </summary>
    /// <param name="arg"></param>
    /// <returns></returns>
    LogicalDeviceBuilder& layerNames(Names&& arg) {
      layernames = std::move(arg);
      return layerNames(layernames);
    }

		/// <summary>
		///
		/// </summary>
		/// <param name="arg"></param>
		/// <returns></returns>
		LogicalDeviceBuilder& extensions(const Names& arg) {
			info.enabledExtensionCount = static_cast<uint32_t>(arg.size());
			if(!arg.empty())info.ppEnabledExtensionNames = arg.data();
			return *this;
		}

    /// <summary>
    ///
    /// </summary>
    /// <param name="arg"></param>
    /// <returns></returns>
    LogicalDeviceBuilder& extensions(Names&& arg) {
      extensionnames = std::move(arg);
      return extensions(extensionnames);
    }

		/// <summary>
		///
		/// </summary>
		/// <param name="arg"></param>
		/// <returns></returns>
		LogicalDeviceBuilder& features(const VkPhysicalDeviceFeatures& arg) {
			info.pEnabledFeatures = &arg;
			return *this;
		}

    /// <summary>
    ///
    /// </summary>
    /// <param name="arg"></param>
    /// <returns></returns>
    LogicalDeviceBuilder& features(VkPhysicalDeviceFeatures&& arg) {
      auto temp = std::move(arg);
      return features(temp);
    }

		/// <summary>
		///
		/// </summary>
		/// <param name="qinfo"></param>
		/// <returns></returns>
		LogicalDeviceBuilder& addQueueInfo(const VkDeviceQueueCreateInfo& qinfo) {
			queueInfos.push_back(qinfo);
			return *this;
		}

    /// <summary>
    ///
    /// </summary>
    /// <param name="qinfo"></param>
    /// <returns></returns>
    LogicalDeviceBuilder& addQueueInfo(VkDeviceQueueCreateInfo&& qinfo) {
      queueInfos.push_back(std::move(qinfo));
      return *this;
    }

  };

  using LogicalDeviceBuilder = LogicalDevice::LogicalDeviceBuilder;

}