#pragma once

#include "vk.h"

#include <memory>

namespace cppvk {

  /**
   * @brief
   *
   * @tparam Container
   */
  template<template<typename E, typename Allocator=std::allocator<E>>typename Container>
  struct PhysicalDeviceDetails {
    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties memoryProperties;
    Container<VkExtensionProperties> extensions;
    Container<VkLayerProperties> validations;
    Container<VkQueueFamilyProperties> queueProperties;

    PhysicalDeviceDetails() = delete;
    explicit PhysicalDeviceDetails(VkPhysicalDevice pDevice) {
      vkGetPhysicalDeviceProperties(pDevice, &properties);
      vkGetPhysicalDeviceFeatures(pDevice, &features);
      vkGetPhysicalDeviceMemoryProperties(pDevice, &memoryProperties);

      uint32_t  count = 0;
      vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &count, nullptr);
      queueProperties.resize(count);
      vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &count, queueProperties.data());

      checkVk(vkEnumerateDeviceExtensionProperties(pDevice, nullptr, &count, nullptr));
      extensions.resize(count);
      checkVk(vkEnumerateDeviceExtensionProperties(pDevice, nullptr, &count, extensions.data()));

      checkVk(vkEnumerateDeviceLayerProperties(pDevice, &count, nullptr));
      validations.resize(count);
      checkVk(vkEnumerateDeviceLayerProperties(pDevice, &count, validations.data()));
    }

    uint32_t findMemoryType(uint32_t filter, VkMemoryPropertyFlags property) {
      for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
        auto type = filter & (1 << i);
        auto flag = memoryProperties.memoryTypes[i].propertyFlags & property;
        if( type && flag == property) return i;
      }
      throw std::runtime_error("Failed to find memory type");
    }
  };

  /**
   * @brief
   *
   * @tparam Container
   */
  template<template<typename E, typename Allocator=std::allocator<E>>typename Container>
  struct PhysicalDeviceSurfaceDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    Container<VkSurfaceFormatKHR> formatList;
    Container<VkPresentModeKHR> presentModeList;

    explicit PhysicalDeviceSurfaceDetails(VkPhysicalDevice pDevice, VkSurfaceKHR pSurface) {
      checkVk(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pDevice, pSurface, &capabilities));

      uint32_t size;
      checkVk(vkGetPhysicalDeviceSurfaceFormatsKHR(pDevice, pSurface, &size, nullptr));
      formatList.resize(size);
			checkVk(vkGetPhysicalDeviceSurfaceFormatsKHR(pDevice, pSurface, &size, formatList.data()));

      checkVk(vkGetPhysicalDeviceSurfacePresentModesKHR(pDevice, pSurface, &size, nullptr));
      presentModeList.resize(size);
      checkVk(vkGetPhysicalDeviceSurfacePresentModesKHR(pDevice, pSurface, &size, presentModeList.data()));
    }
  };
}