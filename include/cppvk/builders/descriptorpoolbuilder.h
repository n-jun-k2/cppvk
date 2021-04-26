#pragma once

#include "../vk.h"
#include "../type.h"
#include "../deleter/deleter.h"

#include "../info/descriptorpoolsizeinfo.h"

namespace cppvk {
  class DescriptorPoolBuilder:
  Noncopyable, Nondynamicallocation  {
    private:
      VkDescriptorPoolCreateInfo m_info;
      cppvk::DeviceRef m_refDevice;

    public:
      explicit DescriptorPoolBuilder(cppvk::DeviceRef refDevice) : m_refDevice(refDevice) {
        m_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        m_info.flags = 0;
        m_info.pNext = nullptr;
      }
      DescriptorPoolBuilder() = delete;

      cppvk::DescriptorPoolPtr create(AllocationCallbacksPtr callbacks = nullptr) {
        if(auto pDevice = m_refDevice.lock()) {
          VkDescriptorPool pool;
          checkVk(vkCreateDescriptorPool(pDevice.get(), &m_info, callbacks ? callbacks.get() : VK_NULL_HANDLE, &pool));
          return DescriptorPoolPtr(pool, DescriptorPoolDeleter(pDevice, callbacks));
        }
        throw std::runtime_error("Failed to DescriptorPool.");
      }

      DescriptorPoolBuilder& flags(VkDescriptorPoolCreateFlags flag) {
        m_info.flags = flag;
        return *this;
      }

      DescriptorPoolBuilder& maxSets(const uint32_t size) {
        m_info.maxSets = size;
        return *this;
      }

      DescriptorPoolBuilder& poolsize(){
        m_info.poolSizeCount = 0;
        m_info.pPoolSizes = nullptr;
        return *this;
      }

      template < template<typename E, typename Allocator = std::allocator<E>>class Container>
      DescriptorPoolBuilder& poolsize(DescriptorPoolSizeList<Container>& list) {
        m_info.poolSizeCount = static_cast<uint32_t>(list.row.size());
        m_info.pPoolSizes = nullptr;
        if (!list.row.empty())
          m_info.pPoolSizes = list.row.data();
        return *this;
      }
  };
}