#pragma once

#include "../vk.h"
#include "../objects/logicaldevice.h"
#include "allocator.h"

namespace cppvk {

  template <typename T>
  class DeviceMemoryAllocate : public cppvk::Allocater<T, VkMemoryAllocateInfo> {
    public:
      explicit DeviceMemoryAllocate(const uint32_t memoryCount, typename cppvk::Allocater<T, VkMemoryAllocateInfo>::AllocateFunc& arg)
        : memoryTypeCount(memoryCount), cppvk::Allocater<T, VkMemoryAllocateInfo>(arg) {
        this->m_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        this->m_info.pNext = VK_NULL_HANDLE;
      }

      ~DeviceMemoryAllocate() = default;
      DeviceMemoryAllocate(const DeviceMemoryAllocate&) = default;
      DeviceMemoryAllocate& operator=(const DeviceMemoryAllocate&) = default;
      DeviceMemoryAllocate(DeviceMemoryAllocate&&) = default;
      DeviceMemoryAllocate& operator=(DeviceMemoryAllocate&&) = default;

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

    private:
      const uint32_t memoryTypeCount;
  };
}