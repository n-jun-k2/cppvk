#pragma once

#include "../vk.h"
#include "../type.h"
#include "../pointer.h"
#include "../deleter/deleter.h"

namespace cppvk {

  class DeviceMemoryAllocate :
    Noncopyable, Nondynamicallocation {
    private:
      VkMemoryAllocateInfo m_info;
      DeviceRef m_refDevice;
    public:
      explicit DeviceMemoryAllocate(DeviceRef refDevice)
      : m_refDevice(refDevice) {
        this->m_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        this->m_info.pNext = VK_NULL_HANDLE;
      }
      ~DeviceMemoryAllocate() = default;

      // DeviceMemoryAllocate& pNext(VkDedicatedAllocationMemoryAllocateInfoNV *pinfo) {
      //   this->m_info.pNext = pinfo;
      //   return *this;
      // }

      DeviceMemoryAllocate& size(const VkDeviceSize size){
        this->m_info.allocationSize = size;
        return *this;
      }

      DeviceMemoryAllocate& typeIndex(const uint32_t index) {
        this->m_info.memoryTypeIndex = index;
        return *this;
      }

      DeviceMemoryPtr allocate(AllocationCallbacksPtr callbacks = nullptr) {
        if(auto pDevice = m_refDevice.lock()) {
          VkDeviceMemory memory;
          checkVk(vkAllocateMemory(pDevice.get(), &m_info, callbacks ? callbacks.get() : VK_NULL_HANDLE, &memory));
          return DeviceMemoryPtr(memory, DeviceMemoryDeleter(pDevice, callbacks));
        }
        throw std::runtime_error("Failed to allocate DeviceMemory");
      }
  };
}