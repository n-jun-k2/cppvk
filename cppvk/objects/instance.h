#pragma once


#include "../vk.h"
#include "../physicaldeviceset.h"
#include "object.h"
#include <cassert>
#include <memory>

#define TO_VKPFN(NAME) PFN_##NAME
#define GetVkInstanceProcAddr(instance, FUNCNAME) \
		(TO_VKPFN(FUNCNAME))vkGetInstanceProcAddr(instance, #FUNCNAME)

namespace cppvk {

  /// <summary>
  /// 
  /// </summary>
  class Instance :public Object {

  public:
    using Object::Object;
    using Ptr = std::shared_ptr<Instance>;

    PhysicalDeviceList getEnumeratePhysicalDevices() {
      assert(this->context->instance != VK_NULL_HANDLE);
      uint32_t size;
      checkVk(vkEnumeratePhysicalDevices(this->context->instance, &size, nullptr));
      PhysicalDeviceList list(size);
      checkVk(vkEnumeratePhysicalDevices(this->context->instance, &size, list.data()));
      return list;
    }

    PhysicalDeviceGroupList getEnumeratePhysicalDeviceGroupsKHR() {
      assert(this->context->instance != VK_NULL_HANDLE);
      uint32_t size = 0;
      checkVk(vkEnumeratePhysicalDeviceGroupsKHR(this->context->instance, &size, nullptr));
      PhysicalDeviceGroupList list(size);
      checkVk(vkEnumeratePhysicalDeviceGroupsKHR(this->context->instance, &size, list.data()));
      return list;
    }

    PhysicalDeviceGroupList getEnumeratePhysicalDeviceGroups() {
      assert(this->context->instance != VK_NULL_HANDLE);
      uint32_t size = 0;
      checkVk(vkEnumeratePhysicalDeviceGroups(this->context->instance, &size, nullptr));
      PhysicalDeviceGroupList list(size);
      checkVk(vkEnumeratePhysicalDeviceGroups(this->context->instance, &size, list.data()));
      return list;
    }

    PhysicalDeviceSet chooseGpu(ChoosePhysicalDeviceFuncInstanceBuilder isSuitable) {

      PhysicalDeviceList gpus = getEnumeratePhysicalDevices();
      if (gpus.empty())
        throw std::runtime_error("failed to find GPUs with vulkan support!");

      for (auto&& gpu : gpus) {
        auto dev_set = PhysicalDeviceSet(gpu);

        if (!isSuitable(dev_set))	continue;

        return dev_set;
      }
      return VK_NULL_HANDLE;
    }

    PFN_vkCreateDebugReportCallbackEXT getCreateDebugReportCallbackEXT() {
      return GetVkInstanceProcAddr(this->context->instance, vkCreateDebugReportCallbackEXT);
    }

    PFN_vkDebugReportMessageEXT getDebugReportMessageEXT() {
      return GetVkInstanceProcAddr(this->context->instance, vkDebugReportMessageEXT);
    }

    PFN_vkDestroyDebugReportCallbackEXT getkDestroyDebugReportCallbackEXT() {
      return GetVkInstanceProcAddr(this->context->instance, vkDestroyDebugReportCallbackEXT);
    }
  };
}