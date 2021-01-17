#pragma once

#include "../vk.h"
#include "allocator.h"

namespace cppvk {

  template<typename T>
  class CommandBufferAllocate : public cppvk::Allocater<T, VkCommandBufferAllocateInfo> {

  public:

    explicit CommandBufferAllocate(typename cppvk::Allocater<T, VkCommandBufferAllocateInfo>::AllocateFunc& arg)
      : cppvk::Allocater<T, VkCommandBufferAllocateInfo>(arg) {
      this->m_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
      this->m_info.pNext = VK_NULL_HANDLE;
    }

    ~CommandBufferAllocate() = default;
    CommandBufferAllocate(const CommandBufferAllocate&) = default;
    CommandBufferAllocate& operator=(const CommandBufferAllocate&) = default;
    CommandBufferAllocate(CommandBufferAllocate&&) = default;
    CommandBufferAllocate& operator=(CommandBufferAllocate&&) = default;

    CommandBufferAllocate& level(const VkCommandBufferLevel& level) {
      this->m_info.level = level;
      return *this;
    }

    CommandBufferAllocate& commandBufferCount(const uint32_t& count) {
      this->m_info.commandBufferCount = count;
      return *this;
    }

  };

}