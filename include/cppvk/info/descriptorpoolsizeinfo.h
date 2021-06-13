#pragma once

#include "../vk.h"
#include "../type.h"

namespace cppvk {
  class DescriptorPoolSizeInfoWrapper {
    private:
      VkDescriptorPoolSize m_info;
    public:
      DescriptorPoolSizeInfoWrapper(VkDescriptorType type, uint32_t count) {
        m_info.descriptorCount = count;
        m_info.type = type;
      }

      static DescriptorPoolSizeInfoWrapper accelerationKHR(uint32_t count) {
        return DescriptorPoolSizeInfoWrapper(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, count);
      }
      static DescriptorPoolSizeInfoWrapper accelerationNV(uint32_t count) {
        return DescriptorPoolSizeInfoWrapper(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV, count);
      }

      static DescriptorPoolSizeInfoWrapper combinedImageSampler(uint32_t count) {
        return DescriptorPoolSizeInfoWrapper(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, count);
      }

      static DescriptorPoolSizeInfoWrapper inlineUniformBlockEXT(uint32_t count) {
        return DescriptorPoolSizeInfoWrapper(VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT, count);
      }

      static DescriptorPoolSizeInfoWrapper inputAttachment(uint32_t count) {
        return DescriptorPoolSizeInfoWrapper(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, count);
      }

      static DescriptorPoolSizeInfoWrapper maxEnum(uint32_t count) {
        return DescriptorPoolSizeInfoWrapper(VK_DESCRIPTOR_TYPE_MAX_ENUM, count);
      }

      static DescriptorPoolSizeInfoWrapper mutableValue(uint32_t count) {
        return DescriptorPoolSizeInfoWrapper(VK_DESCRIPTOR_TYPE_MUTABLE_VALVE, count);
      }

      static DescriptorPoolSizeInfoWrapper sampledImage(uint32_t count) {
        return DescriptorPoolSizeInfoWrapper(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, count);
      }

      static DescriptorPoolSizeInfoWrapper sampler(uint32_t count) {
        return DescriptorPoolSizeInfoWrapper(VK_DESCRIPTOR_TYPE_SAMPLER, count);
      }

      static DescriptorPoolSizeInfoWrapper storageBuffer(uint32_t count) {
        return DescriptorPoolSizeInfoWrapper(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, count);
      }

      static DescriptorPoolSizeInfoWrapper storageBufferDynamic(uint32_t count) {
        return DescriptorPoolSizeInfoWrapper(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, count);
      }

      static DescriptorPoolSizeInfoWrapper storageImage(uint32_t count) {
        return DescriptorPoolSizeInfoWrapper(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, count);
      }

      static DescriptorPoolSizeInfoWrapper storageTexelBuffer(uint32_t count) {
        return DescriptorPoolSizeInfoWrapper(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, count);
      }

      static DescriptorPoolSizeInfoWrapper uniformBuffer(uint32_t count) {
        return DescriptorPoolSizeInfoWrapper(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, count);
      }

      static DescriptorPoolSizeInfoWrapper uniformBufferDynamic(uint32_t count) {
        return DescriptorPoolSizeInfoWrapper(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, count);
      }

      static DescriptorPoolSizeInfoWrapper uniformTexelBuffer(uint32_t count) {
        return DescriptorPoolSizeInfoWrapper(VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, count);
      }
  };

  template < template<typename E, typename Allocator = std::allocator<E>>class Container>
  using DescriptorPoolSizeList = cppvk::WrapContainer< VkDescriptorPoolSize, DescriptorPoolSizeInfoWrapper, Container>;


}