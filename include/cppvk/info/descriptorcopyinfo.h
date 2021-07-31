#pragma once

#include "../vk.h"
#include "../type.h"

namespace cppvk {

  class DescriptorCopyInfoWrapper {
    private:
      VkCopyDescriptorSet info;

    public:
      DescriptorCopyInfoWrapper() {
        info.sType = VK_STRUCTURE_TYPE_COPY_DESCRIPTOR_SET;
        info.pNext = VK_NULL_HANDLE;
      }

      template<uint64_t S>
      DescriptorCopyInfoWrapper& descriptor(std::array<VkDescriptorSet, S>* src, std::array<VkDescriptorSet, S>* dst) {
        info.descriptorCount = S;
        info.srcSet = *src->data();
        info.dstSet = *dst->data();
        return *this;
      }

      DescriptorCopyInfoWrapper& srcBinding(uint32_t src) {
        info.srcBinding = src;
        return *this;
      }

      DescriptorCopyInfoWrapper& srcArrayElement(uint32_t src) {
        info.srcArrayElement = src;
        return *this;
      }

      DescriptorCopyInfoWrapper& dstBinding(uint32_t dst) {
        info.dstBinding = dst;
        return *this;
      }

      DescriptorCopyInfoWrapper& dstArrayElement(uint32_t dst) {
        info.dstArrayElement = dst;
        return *this;
      }
  };

  template < template<typename E, typename Allocator = std::allocator<E>>class Container>
  using DescriptorCopyInfoList = cppvk::WrapContainer< VkCopyDescriptorSet, DescriptorCopyInfoWrapper, Container>;

}