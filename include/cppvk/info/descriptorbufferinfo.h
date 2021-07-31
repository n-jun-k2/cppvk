#pragma once

#include "../vk.h"
#include "../pointer.h"

namespace cppvk {

  class DescriptorBufferInfoWrapper {
    private:
      VkDescriptorBufferInfo info;
    public:
      DescriptorBufferInfoWrapper()
      {
        info.buffer = NULL;
        info.offset = 0;
        info.range = 0;
      }

      DescriptorBufferInfoWrapper& buffer(VkBuffer buffer) {
        info.buffer = buffer;
        return *this;
      }

      DescriptorBufferInfoWrapper& buffer(cppvk::BufferPtr buffer) {
        info.buffer = buffer.get();
        return *this;
      }

      DescriptorBufferInfoWrapper& offset(const uint32_t offset) {
        info.offset = offset;
        return *this;
      }

      DescriptorBufferInfoWrapper& range(const uint32_t range) {
        info.range = range;
        return *this;
      }
  };

  template < template<typename E, typename Allocator = std::allocator<E>>class Container>
  using DescriptorBufferInfoList = cppvk::WrapContainer< VkDescriptorBufferInfo, DescriptorBufferInfoWrapper, Container>;


}