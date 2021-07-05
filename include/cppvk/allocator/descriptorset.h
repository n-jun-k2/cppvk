#pragma once

#include "../vk.h"
#include "../type.h"
#include "../pointer.h"
#include "../deleter/deleter.h"
#include "../poolobject.h"

namespace cppvk {

  template<size_t Length>
  class DescriptorSetAllocate :
  Noncopyable, Nondynamicallocation {
  private:
    VkDescriptorSetAllocateInfo m_info;
    DeviceRef m_refDevice;
    DescriptorPoolRef m_refDescriptorPool;
    DescriptorSetLayoutPoolRef m_refDescriptorSetLayoutPool;
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

    DescriptorSetAllocate& layoutpool(DescriptorSetLayoutPoolRef pool) {
      m_refDescriptorSetLayoutPool = pool;
      return *this;
    }

    DescriptorSetPtr<Length> allocate() {
      auto pDevice = this->m_refDevice.lock();
      auto pPool = this->m_refDescriptorPool.lock();
      auto pLayoutPool = this->m_refDescriptorSetLayoutPool.lock();
      if(pDevice && pPool && pLayoutPool){
        m_info.descriptorPool = pPool.get();
        m_info.descriptorSetCount = static_cast<uint32_t>(pLayoutPool->size());
        m_info.pSetLayouts = nullptr;
        if(m_info.descriptorSetCount > 0) {
          m_info.pSetLayouts = pLayoutPool->data();
        }

        auto buffer = new std::array<VkDescriptorSet, Length>();
        checkVk(vkAllocateDescriptorSets(pDevice.get(), &m_info, buffer->data()));
        return DescriptorSetPtr<Length>(buffer, cppvk::DescriptorSetDeleter<Length>(std::make_shared<device_and_descriptorpool>(pDevice, pPool), nullptr));
      }
      throw std::runtime_error("Failed to create descriptorset");
    }

  };
}