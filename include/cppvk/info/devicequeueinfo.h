#pragma once

#include "../vk.h"
#include "../type.h"

namespace cppvk {

  class DeviceQueueCreateInfoWrapper {
  private:
    VkDeviceQueueCreateInfo info;
  public:

    DeviceQueueCreateInfoWrapper() {
      info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      info.pNext = NULL;
      info.flags = 0;
    }

    /// <summary>
    ///
    /// </summary>
    /// <param name="index"></param>
    /// <returns></returns>
    DeviceQueueCreateInfoWrapper& familyIndex(const uint32_t index) {
      info.queueFamilyIndex = index;
      return *this;
    }

    /// <summary>
    ///
    /// </summary>
    /// <param name="priorities"></param>
    /// <returns></returns>
    template<template<typename T, typename Allocator = std::allocator<T>> class Container>
    DeviceQueueCreateInfoWrapper& queuePriorities(const Container<float>& priorities) {
      info.queueCount = static_cast<uint32_t>(priorities.size());
      if (!priorities.empty()) { // deep copy
        info.pQueuePriorities = new float[info.queueCount];
        auto ptr = std::remove_const_t<float*>(info.pQueuePriorities);
        std::copy(
          std::begin(priorities),
          std::end(priorities),
          ptr
        );
      }
      return *this;
    }

    /// <summary>
    ///
    /// </summary>
    /// <param name="priorities"></param>
    /// <returns></returns>
    template<template<typename T, typename Allocator = std::allocator<T>> class Container>
    DeviceQueueCreateInfoWrapper& queuePriorities(Container<float>&& priorities) {
      auto temp = std::move(priorities);
      return queuePriorities(temp);
    }

  };

 template < template<typename E, typename Allocator = std::allocator<E>>class Container>
 using DeviceQueueCreateInfoList = cppvk::UnionWrapper< VkDeviceQueueCreateInfo, DeviceQueueCreateInfoWrapper, Container>;

}