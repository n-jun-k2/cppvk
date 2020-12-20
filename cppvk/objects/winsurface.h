#pragma once

#include "../vk.h"
#include "../context.h"
#include "object.h"

#include <memory>


namespace cppvk {

  class WinSurface : public Object {

    public:
      using Object::Object;
      using Ptr = std::shared_ptr<WinSurface>;


      /// <summary>
      /// Obtaining surface stats
      /// </summary>
      /// <param name="gpu"></param>
      /// <returns></returns>
      VkSurfaceCapabilitiesKHR GetSurfaceCapabilities(VkPhysicalDevice gpu)
      {
        VkSurfaceCapabilitiesKHR temp{};
        cppvk::checkVk(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, this->context->surface, &temp));
        return temp;
      }

      /// <summary>
      /// Obtaining surface stats
      /// </summary>
      /// <returns></returns>
      VkSurfaceCapabilitiesKHR GetSurfaceCapabilities() {
        return GetSurfaceCapabilities(this->context->physicaldevice);
      }

      /// <summary>
      /// Display formats that can be used on the surface of physical devices
      /// </summary>
      /// <param name="gpu"></param>
      /// <returns></returns>
      SurfaceFormats GetEnumerateSurfaceFormats(VkPhysicalDevice gpu)
      {
        uint32_t fcount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, this->context->surface, &fcount, nullptr);
        SurfaceFormats formats(fcount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, this->context->surface, &fcount, formats.data());
        return formats;
      }


      /// <summary>
      /// Display formats that can be used on the surface of physical devices
      /// </summary>
      /// <returns></returns>
      SurfaceFormats GetEnumerateSurfaceFormats() {
        return GetEnumerateSurfaceFormats(this->context->physicaldevice);
      }

      /// <summary>
      /// Present mode where the surface of the physical device can be used
      /// Find out how to switch between frontand back screens(called Present).
      /// </summary>
      /// <param name="gpu"></param>
      /// <returns></returns>
      PresentModes GetEnumerateSurfacePresentmodes(VkPhysicalDevice gpu)
      {
        uint32_t pcount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, this->context->surface, &pcount, nullptr);
        PresentModes presentModes(pcount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, this->context->surface, &pcount, presentModes.data());
        return presentModes;
      }

      /// <summary>
      /// Present mode where the surface of the physical device can be used
      /// Find out how to switch between frontand back screens(called Present).
      /// </summary>
      /// <returns></returns>
      PresentModes GetEnumerateSurfacePresentmodes() {
        return GetEnumerateSurfacePresentmodes(this->context->physicaldevice);
      }

      /// <summary>
      /// Check if the surface function of this device is supported.
      /// </summary>
      /// <param name="gpu"></param>
      /// <param name="index"></param>
      /// <returns></returns>
      bool GetPhysicalDevicceSurfaceSupportKHR(VkPhysicalDevice gpu, const uint32_t index) {
        VkBool32 isSupport = false;
        cppvk::checkVk(vkGetPhysicalDeviceSurfaceSupportKHR(gpu, index, this->context->surface, &isSupport));
        return isSupport == VK_TRUE;
      }

      /// <summary>
      /// Check if the surface function of this device is supported.
      /// </summary>
      /// <param name="index"></param>
      /// <returns></returns>
      bool GetPhysicalDevicceSurfaceSupportKHR(const uint32_t index) {
        return GetPhysicalDevicceSurfaceSupportKHR(this->context->physicaldevice, index);
      }
  };

}