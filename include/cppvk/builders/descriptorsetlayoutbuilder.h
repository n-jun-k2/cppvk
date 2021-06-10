#pragma once

#include "../vk.h"
#include "../type.h"
#include "../pointer.h"
#include "../deleter/deleter.h"

#include "../info/descriptorsetlayoutbindinginfo.h"

#include <vector>
#include <tuple>

namespace cppvk {

  class DescriptorSetLayoutBuilder :
    Noncopyable, Nondynamicallocation {
    private:
      VkDescriptorSetLayoutCreateInfo m_info;
      DeviceRef m_refDevice;

    public:
      explicit DescriptorSetLayoutBuilder(DeviceRef refDevice):m_refDevice(refDevice){
        m_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        m_info.flags = 0;
        m_info.pNext = nullptr;
      }
      DescriptorSetLayoutBuilder() = delete;

      cppvk::DescriptorSetLayoutPtr create(AllocationCallbacksPtr callbacks = nullptr) {
        if (auto pDevice = m_refDevice.lock()) {
          VkDescriptorSetLayout layout;
          checkVk(vkCreateDescriptorSetLayout(pDevice.get(), &m_info, callbacks ? callbacks.get() : VK_NULL_HANDLE, &layout));
          return DescriptorSetLayoutPtr(layout, DescriptorSetLayoutDeleter(pDevice, callbacks));
        }
        throw std::runtime_error("Fialed to create DescritptorSetLayout");
      }

      DescriptorSetLayoutBuilder& flags(VkDescriptorSetLayoutCreateFlags flag) {
        m_info.flags = flag;
        return *this;
      }

      template< template<typename T, typename Allocator = std::allocator<T>> class Container>
      DescriptorSetLayoutBuilder& bindings(DescriptorSetLayoutBindingList<Container>& list) {
        m_info.bindingCount = static_cast<uint32_t>(list.row.size());
        m_info.pBindings = nullptr;
        if (!list.row.empty())
          m_info.pBindings = list.row.data();
        return *this;
      }
  };
}