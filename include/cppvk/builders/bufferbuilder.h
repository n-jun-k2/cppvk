#pragma once

#include "../vk.h"
#include "../type.h"
#include "../pointer.h"
#include "../deleter/deleter.h"

namespace cppvk {
  class BufferBuilder :Noncopyable, Nondynamicallocation {
    private:
      VkBufferCreateInfo m_info;
      cppvk::DeviceRef m_refLogicalDevice;

    public:
      explicit BufferBuilder(DeviceRef pLogicalDevice)
      : m_refLogicalDevice(pLogicalDevice) {
        m_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        m_info.pNext = VK_NULL_HANDLE;
        m_info.flags = 0;
        m_info.queueFamilyIndexCount = 0;
        m_info.pQueueFamilyIndices = nullptr;
        m_info.size = 0;
      }

      cppvk::BufferPtr create(AllocationCallbacksPtr callbacks = nullptr) {
        if(auto pLogicalDevice = m_refLogicalDevice.lock()) {
          VkBuffer buffer;
          checkVk(vkCreateBuffer(pLogicalDevice.get(), &m_info, callbacks ? callbacks.get() : VK_NULL_HANDLE, &buffer));
          return BufferPtr(buffer, BufferDeleter(pLogicalDevice, callbacks));
        }
        throw std::runtime_error("Failed to create Buffer");
      }

      BufferBuilder& next() {
        m_info.pNext = VK_NULL_HANDLE;
        return *this;
      }

      BufferBuilder& flags(VkBufferCreateFlagBits flags) {
        m_info.flags = static_cast<VkBufferCreateFlags>(flags);
        return *this;
      }

      BufferBuilder& usage(VkBufferUsageFlagBits flags) {
        m_info.usage = static_cast<VkBufferUsageFlags>(flags);
        return *this;
      }

      BufferBuilder& size(const size_t size) {
        m_info.size = size;
        return *this;
      }

      BufferBuilder& queueFamilyIndices(cppvk::Indexs& indices) {
        m_info.queueFamilyIndexCount = static_cast<uint32_t>(indices.size());
        m_info.pQueueFamilyIndices = nullptr;
        if (!indices.empty())
          m_info.pQueueFamilyIndices = indices.data();
        return *this;
      }

      BufferBuilder& sharingMode(VkSharingMode mode) {
        m_info.sharingMode = mode;
        return *this;
      }

  };
}