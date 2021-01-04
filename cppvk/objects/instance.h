#pragma once

#include "../vk.h"
#include "../destroy.h"
#include "object.h"
#include <functional>

namespace cppvk {

  class Instance : public cppvk::Object {

    public:
      class InstanceBuilder;
      class DebugUtilsMessenger;
      class PhysicalDevice;
      class Surface;

      using Object::Object;
      using pointer = std::shared_ptr<Instance>;
      using reference = std::weak_ptr<Instance>;

      PhysicalDeviceList getEnumeratePhysicalDevices() {
        PhysicalDeviceList list;
        getEnumerate<VkInstance, VkPhysicalDevice>(instance, list, vkEnumeratePhysicalDevices);
        return list;
      }

      PhysicalDeviceGroupList getEnumeratePhysicalDeviceGroupsKHR() {
        PhysicalDeviceGroupList list;
        getEnumerate<VkInstance, VkPhysicalDeviceGroupPropertiesKHR>(instance, list, vkEnumeratePhysicalDeviceGroupsKHR);
        return list;
      }

      PhysicalDeviceGroupList getEnumeratePhysicalDeviceGroups() {
        PhysicalDeviceGroupList list;
        getEnumerate<VkInstance, VkPhysicalDeviceGroupProperties>(instance, list, vkEnumeratePhysicalDeviceGroups);
        return list;
      }

    private:
      VkInstance instance;
      std::unique_ptr<Destroy> destroy;
  };

}