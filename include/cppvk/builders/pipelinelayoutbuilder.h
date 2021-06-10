#pragma once

#include "../vk.h"
#include "../type.h"
#include "../pointer.h"
#include "../deleter/deleter.h"

namespace cppvk {
  class PipelineLayoutBuilder :
  Noncopyable, Nondynamicallocation{
    private:
      VkPipelineLayoutCreateInfo m_info;
      cppvk::DeviceRef m_refDevice;

    public:
      explicit PipelineLayoutBuilder(cppvk::DeviceRef refDevice) : m_refDevice(refDevice) {
        m_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        m_info.pNext = VK_NULL_HANDLE;
        m_info.flags = 0;
      }
      PipelineLayoutBuilder() = delete;

      cppvk::PipelineLayoutPtr create(AllocationCallbacksPtr callbacks = nullptr) {
        if (auto pDevice = m_refDevice.lock()) {
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

      PipelineLayoutBuilder& layouts(){
        m_info.setLayoutCount = 0;
        m_info.pSetLayouts = nullptr;
        return *this;
      }

      template< template<class T, class Allocator = std::allocator<T>> class Container>
      PipelineLayoutBuilder& layouts(Container<VkDescriptorSetLayout>& pLayouts) {
        m_info.setLayoutCount = static_cast<uint32_t>(pLayouts.size());
        m_info.pSetLayouts = nullptr;
        if (m_info.setLayoutCount > 0)
          m_info.pSetLayouts = pLayouts.data();
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