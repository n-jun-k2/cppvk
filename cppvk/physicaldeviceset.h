#pragma once
#include "vk.h"
#include <cassert>

namespace cppvk {
  struct PhysicalDeviceSet;

  using ChoosePhysicalDeviceFuncInstanceBuilder = std::function<bool(PhysicalDeviceSet&)>;

  struct PhysicalDeviceSet {
    VkPhysicalDevice device;
    VkPhysicalDeviceProperties props;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties memory;
    ExtensionPropertiesList extensions;
    LayerPropertiesList validations;
    PhysicalDeviceQueueProps qprops;

    PhysicalDeviceSet(VkPhysicalDevice gpu) :device(gpu) {
      vkGetPhysicalDeviceProperties(gpu, &props);
      vkGetPhysicalDeviceFeatures(gpu, &features);
      vkGetPhysicalDeviceMemoryProperties(gpu, &memory);
      extensions = getEnumeratePhysicalDeviceExtensions(gpu);
      validations = getEnumerateDeviceLayer(gpu);
      qprops = getEnumeratePhysicalDeviceQueueProps(gpu);
    }

    PhysicalDeviceQueueProps getEnumeratePhysicalDeviceQueueProps(const VkPhysicalDevice gpu) {
      assert(gpu != VK_NULL_HANDLE);
      uint32_t  count = 0;
      vkGetPhysicalDeviceQueueFamilyProperties(gpu, &count, nullptr);
      PhysicalDeviceQueueProps queue(count);
      vkGetPhysicalDeviceQueueFamilyProperties(gpu, &count, queue.data());
      return queue;
    }

    ExtensionPropertiesList getEnumeratePhysicalDeviceExtensions(const VkPhysicalDevice gpu) {
      assert(gpu != VK_NULL_HANDLE);
      uint32_t size = 0;
      checkVk(vkEnumerateDeviceExtensionProperties(gpu, nullptr, &size, nullptr));
      ExtensionPropertiesList list(size);
      checkVk(vkEnumerateDeviceExtensionProperties(gpu, nullptr, &size, list.data()));
      return list;
    }

    LayerPropertiesList getEnumerateDeviceLayer(const VkPhysicalDevice gpu) {
      assert(gpu != VK_NULL_HANDLE);
      uint32_t size = 0;
      checkVk(vkEnumerateDeviceLayerProperties(gpu, &size, nullptr));
      LayerPropertiesList list(size);
      checkVk(vkEnumerateDeviceLayerProperties(gpu, &size, list.data()));
      return list;
    }

  };

}