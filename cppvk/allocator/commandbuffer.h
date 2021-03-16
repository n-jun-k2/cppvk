#pragma once

#include "../vk.h"
#include "../type.h"
#include "../pointer.h"
#include "../deleter/deleter.h"

namespace cppvk {

  template<size_t Length>
  class CommandBufferAllocate :
  Noncopyable, Nondynamicallocation{
  private:
    VkCommandBufferAllocateInfo m_info;
    DeviceRef m_refDevice;
    CommandPoolRef m_refCommandPool;
  public:

    explicit CommandBufferAllocate(DeviceRef refDevice) : m_refDevice(refDevice){
      m_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
      m_info.pNext = VK_NULL_HANDLE;
      m_info.commandBufferCount = Length;
    }
    ~CommandBufferAllocate() = default;

    CommandBufferAllocate& level(const VkCommandBufferLevel& level) {
      m_info.level = level;
      return *this;
    }

    CommandBufferAllocate& commandPool(CommandPoolRef refCommandPool) {
      m_refCommandPool = refCommandPool;
      return *this;
    }

    CommandBufferPtr<Length> allocate() {
      auto pCmdPool = this->m_refCommandPool.lock();
      auto pDevice = this->m_refDevice.lock();
      if (pDevice && pCmdPool ) {
        this->m_info.commandPool = pCmdPool.get();
        auto buffer = new std::array<VkCommandBuffer, Length> { VK_NULL_HANDLE };
        cppvk::checkVk(vkAllocateCommandBuffers(pDevice.get(), &m_info, buffer->data()));
        return CommandBufferPtr<Length>(buffer, cppvk::CommandBufferDeleter<Length>(std::make_shared<deivce_and_commandpool>(pDevice, pCmdPool), VK_NULL_HANDLE));
      }
      throw std::runtime_error("Failed to create CommandBuffers");
    }

  };

}