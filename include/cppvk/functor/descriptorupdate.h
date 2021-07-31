#pragma once

#include "../vk.h"
#include "../pointer.h"
#include "../info/descriptorwriteinfo.h"
#include "../info/descriptorcopyinfo.h"

#include <vector>
#include <functional>

namespace cppvk {

  template<uint32_t _DescriptorPoolSize>
  class DescriptorUpdateHelper {
    public:
      using WriteInfoList = DescriptorWriteInfoList<std::vector>;
      using CopyInfoList = DescriptorCopyInfoList<std::vector>;
    private:
      DeviceRef m_refDevice;
      DescriptorSetRef<_DescriptorPoolSize> m_refSrcDescriptor;
      DescriptorSetRef<_DescriptorPoolSize> m_refDstDescriptor;
      WriteInfoList* m_writeList;
      CopyInfoList* m_copyList;

    public:
      DescriptorUpdateHelper() = delete;

      explicit DescriptorUpdateHelper(cppvk::DeviceRef refDevice)
      : m_refDevice(refDevice), m_writeList(nullptr), m_copyList(nullptr)
        {}

      DescriptorUpdateHelper& srcSet(DescriptorSetRef<_DescriptorPoolSize> refSrcDescriptor) {
        m_refSrcDescriptor = refSrcDescriptor;
        return *this;
      }

      DescriptorUpdateHelper& dstSet(DescriptorSetRef<_DescriptorPoolSize> refDstDescriptor) {
        m_refDstDescriptor = refDstDescriptor;
        return *this;
      }

      DescriptorUpdateHelper& write(WriteInfoList& list) {
        m_writeList = &list;
        return *this;
      }

      DescriptorUpdateHelper& copy(CopyInfoList& list) {
        m_copyList = &list;
        return *this;
      }

      void update() {
        auto pDevice = m_refDevice.lock();
        auto pSrcDescriptor = m_refSrcDescriptor.lock();
        auto pDstDescriptor = m_refDstDescriptor.lock();

        auto writeSize = 0;
        VkWriteDescriptorSet* writeData = nullptr;

        auto copySize = 0;
        VkCopyDescriptorSet* copyData = nullptr;

        const auto isCopy = m_copyList != nullptr && !m_copyList->raw.empty() && pSrcDescriptor && pDstDescriptor;
        const auto isWrite = m_writeList != nullptr && !m_writeList->raw.empty() && pDstDescriptor;
        if(isCopy) {
          copySize = static_cast<uint32_t>(m_copyList->raw.size());
          copyData = m_copyList->raw.data();
          for (auto& v : m_copyList->values)
            v.descriptor(pSrcDescriptor.get(), pDstDescriptor.get());
        }

        if(isWrite) {
          writeSize = static_cast<uint32_t>(m_writeList->raw.size());
          writeData = m_writeList->raw.data();
          for (auto& w : m_writeList->values)
            w.dstDescriptor(pDstDescriptor.get());
        }

        if (!pDevice || (!isCopy && !isWrite))return;

        vkUpdateDescriptorSets(pDevice.get(), writeSize, writeData, copySize, copyData);
      }

  };
}