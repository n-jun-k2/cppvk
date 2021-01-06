#pragma once

#include "../vk.h"

namespace cppvk {

  struct PhysicalDeviceSurfaceDetails {

    VkSurfaceCapabilitiesKHR capabilities;
    SurfaceFormats formatList;
    PresentModes presentModeList;

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