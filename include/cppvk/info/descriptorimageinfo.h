#pragma once

#include "../vk.h"
#include "../pointer.h"

namespace cppvk {

  class DescriptorImageInfoWrapper {
    private:
      VkDescriptorImageInfo info;
    public:
      DescriptorImageInfoWrapper() :info({}) {}

      DescriptorImageInfoWrapper& layout(VkImageLayout layout) {
        info.imageLayout = layout;
        return *this;
      }

      DescriptorImageInfoWrapper& sampler(VkSampler sampler) {
        info.sampler = sampler;
        return *this;
      }

      DescriptorImageInfoWrapper view(VkImageView view) {
        info.imageView = view;
        return *this;
      }
  };

  template < template<typename E, typename Allocator = std::allocator<E>>class Container>
  using DescriptorImageInfoList = cppvk::WrapContainer< VkDescriptorImageInfo, DescriptorImageInfoWrapper, Container>;


}