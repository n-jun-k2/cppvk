#pragma once

#include "../vk.h"
#include "info.h"

#include <iterator>
#include <algorithm>

namespace cppvk {

  /// <summary>
  /// 
  /// </summary>
  class DeviceQueueInfo : public cppvk::Info<VkDeviceQueueCreateInfo> {

    public:

      DeviceQueueInfo() {
        info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        info.pNext = NULL;
        info.flags = 0;
      }

      ~DeviceQueueInfo()                                  = default;
      DeviceQueueInfo(const DeviceQueueInfo&)             = default;
      DeviceQueueInfo& operator=(const DeviceQueueInfo&)  = default;
      DeviceQueueInfo(DeviceQueueInfo&&)                  = default;
      DeviceQueueInfo& operator=(DeviceQueueInfo&&)       = default;

      /// <summary>
      /// 
      /// </summary>
      /// <param name="index"></param>
      /// <returns></returns>
      DeviceQueueInfo& familyIndex (const uint32_t index) {
        info.queueFamilyIndex = index;
        return *this;
      }

      /// <summary>
      /// 
      /// </summary>
      /// <param name="priorities"></param>
      /// <returns></returns>
      DeviceQueueInfo& queuePriorities(const Priorities& priorities) {
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
      DeviceQueueInfo& queuePriorities(Priorities&& priorities) {
        auto temp = std::move(priorities);
        return queuePriorities(temp);
      }

  };

}