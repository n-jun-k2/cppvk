#pragma once

#include "../vk.h"
#include "../type.h"
#include "../pointer.h"
#include "../poolobject.h"
#include "../deleter/deleter.h"

namespace cppvk {
  class PipelineLayoutBuilder :
  Noncopyable, Nondynamicallocation{
    private:
      VkPipelineLayoutCreateInfo m_info;
      cppvk::DeviceRef m_refDevice;
      DescriptorSetLayoutPoolRef m_refPool;

    public:
      explicit PipelineLayoutBuilder(cppvk::DeviceRef refDevice) : m_refDevice(refDevice) {
        m_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        m_info.pNext = VK_NULL_HANDLE;
        m_info.flags = 0;
      }
      PipelineLayoutBuilder() = delete;

      cppvk::PipelineLayoutPtr create(AllocationCallbacksPtr callbacks = nullptr) {
        auto pPool = m_refPool.lock();
        auto pDevice = m_refDevice.lock();
        if (pDevice && pPool) {

          m_info.setLayoutCount = static_cast<uint32_t>(pPool->size());
          m_info.pSetLayouts = nullptr;
          if (m_info.setLayoutCount > 0) {
            m_info.pSetLayouts = pPool->data();
          }

          VkPipelineLayout layout;
          checkVk(vkCreatePipelineLayout(pDevice.get(), &m_info,  callbacks ? callbacks.get() : VK_NULL_HANDLE, &layout));
          return PipelineLayoutPtr(layout, PipelineLayoutDeleter(pDevice, callbacks));
        }
        throw std::runtime_error("Failed to create PipelineLayout");
      }

      PipelineLayoutBuilder& flags(VkPipelineLayoutCreateFlags flag) {
        m_info.flags = flag;
        return *this;
      }

      PipelineLayoutBuilder& layoutpool(DescriptorSetLayoutPoolPtr ref) {
        m_refPool = ref;
        return *this;
      }

      PipelineLayoutBuilder& pushConstantRange() {
        m_info.pushConstantRangeCount = 0;
        m_info.pPushConstantRanges = nullptr;
        return *this;
      }

      template< template<class T, class Allocator = std::allocator<T>> class Container>
      PipelineLayoutBuilder& pushConstantRange(Container<VkPushConstantRange>& pRanges) {
        m_info.pushConstantRangeCount = static_cast<uint32_t>(pRanges.size());
        m_info.pPushConstantRanges = nullptr;
        if (m_info.pushConstantRangeCount > 0)
          m_info.pPushConstantRanges = pRanges.data();
        return *this;
      }

  };
}