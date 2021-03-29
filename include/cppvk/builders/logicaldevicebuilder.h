#pragma once

#include "../vk.h"
#include "../type.h"
#include "../pointer.h"
#include "../deleter/deleter.h"
#include "../info/devicequeueinfo.h"
#include <iostream>
#include <stdexcept>

namespace cppvk {

  /// <summary>
  /// Logical device builder pattern
  /// </summary>
  class LogicalDeviceBuilder :
  Noncopyable, Nondynamicallocation {

  private:

    VkDeviceCreateInfo m_info;
    VkPhysicalDevice m_physicalDevice;

    cppvk::Names layernames;
    cppvk::Names extensionnames;
    VkPhysicalDeviceFeatures tempFeatures;

  public:

    LogicalDeviceBuilder() = delete;

    /// <summary>
    /// LogicalDeviceBuilder constructor
    /// </summary>
    /// <param name="physicaldevice"></param>
    explicit LogicalDeviceBuilder(const VkPhysicalDevice physicaldevice) :
      m_physicalDevice(physicaldevice),
      tempFeatures({}) {
      m_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
      m_info.pNext = NULL;
      m_info.flags = 0;
      m_info.queueCreateInfoCount = 0;
      m_info.enabledLayerCount = 0;
      m_info.enabledExtensionCount = 0;
    }

    /// <summary>
    /// Create smart pointer
    /// </summary>
    /// <param name="callbacks"></param>
    /// <returns></returns>
    cppvk::DevicePtr create(AllocationCallbacksPtr callbacks = nullptr) {
      VkDevice vkDevice;
      vkCreateDevice(m_physicalDevice, &m_info, callbacks ? callbacks.get() : VK_NULL_HANDLE, &vkDevice);
      return DevicePtr(vkDevice, LogicalDeviceDeleter(callbacks));
    }

		/// <summary>
		///
		/// </summary>
		/// <param name="layers"></param>
		/// <returns></returns>
		LogicalDeviceBuilder& layerNames(const Names& arg) {
			m_info.enabledLayerCount = static_cast<uint32_t>(arg.size());
			if(!arg.empty())m_info.ppEnabledLayerNames = arg.data();
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
			m_info.enabledExtensionCount = static_cast<uint32_t>(arg.size());
			if(!arg.empty())m_info.ppEnabledExtensionNames = arg.data();
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
			m_info.pEnabledFeatures = &arg;
			return *this;
		}

    /// <summary>
    ///
    /// </summary>
    /// <param name="arg"></param>
    /// <returns></returns>
    LogicalDeviceBuilder& features(VkPhysicalDeviceFeatures&& arg) {
      tempFeatures = std::move(arg);
      return features(tempFeatures);
    }

    template < template<typename E, typename Allocator = std::allocator<E>>class Container>
    LogicalDeviceBuilder& queueCreateInfos(const DeviceQueueCreateInfoList<Container>& arg) {
      m_info.queueCreateInfoCount = static_cast<uint32_t>(arg.row.size());
      if (!arg.row.empty())
        m_info.pQueueCreateInfos = arg.row.data();
      return *this;
    }

  };

}