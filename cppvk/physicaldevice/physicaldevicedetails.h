#pragma once

#include "../vk.h"

namespace cppvk {

  struct PhysicalDeviceDetails {

    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties memoryProperties;
    ExtensionPropertiesList extensions;
    LayerPropertiesList validations;
    PhysicalDeviceQueueProps queueProperties;

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

  };

}