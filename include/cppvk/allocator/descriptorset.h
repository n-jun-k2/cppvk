#pragma once

#include "../vk.h"
#include "../type.h"
#include "../pointer.h"
#include "../deleter/deleter.h"

namespace cppvk {

  template<size_t Length>
  class DescriptorSetAllocate :
  Noncopyable, Nondynamicallocation {
  private:
    VkDescriptorSetAllocateInfo m_info;
    DeviceRef m_refDevice;
    DescriptorPoolRef m_refDescriptorPool;
  public:
    explicit DescriptorSetAllocate(DeviceRef refDevice):m_refDevice(refDevice) {
      m_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
      m_info.pNext = VK_NULL_HANDLE;
    }
    DescriptorSetAllocate() = delete;

    DescriptorSetAllocate& descriptorPool(DescriptorPoolPtr pool) {
      m_refDescriptorPool = pool;
      return *this;
    }

    DescriptorSetAllocate& layouts() {
      m_info.descriptorSetCount = 0;
      m_info.pSetLayouts = nullptr;
      return *this;
    }

    template< template<typename T, typename Allocator = std::allocator<T>> class Container>
    DescriptorSetAllocate& layouts(Container<VkDescriptorSetLayout>& layouts) {
      m_info.descriptorSetCount = static_cast<uint32_t>(layouts.size());
      m_info.pSetLayouts = nullptr;
      if(m_info.descriptorSetCount > 0)
        m_info.pSetLayouts = layouts.data();
      return *this;
    }

    DescriptorSetPtr<Length> allocate() {
      m_info.descriptorSetCount = Length;
      auto pPool = this->m_refDescriptorPool.lock();
      auto pDevice = m_refDevice.lock();
      if(pDevice && pPool){
        m_info.descriptorPool = pPool.get();
        auto buffer = new std::array<VkDescriptorSet, Length>();
        checkVk(vkAllocateDescriptorSets(pDevice.get(), &m_info, buffer->data()));
        return DescriptorSetPtr<Length>(buffer, cppvk::DescriptorSetDeleter<Length>(std::make_shared<device_and_descriptorpool>(pDevice, pPool), nullptr));
      }
      throw std::runtime_error("Failed to create descriptorset");
    }

  };
}