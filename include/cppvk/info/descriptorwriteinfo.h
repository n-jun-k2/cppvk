#pragma once

#include "../vk.h"
#include "../type.h"
#include "../info/descriptorbufferinfo.h"
#include "../info/descriptorimageinfo.h"

namespace cppvk{

  class DescriptorWriteInfoWrapper {
    private:
      VkWriteDescriptorSet info;
    public:

      DescriptorWriteInfoWrapper() {
        info.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        info.pNext = NULL;
      }

      template<uint64_t S>
      DescriptorWriteInfoWrapper& dstDescriptor(std::array<VkDescriptorSet, S>* dst) {
        info.descriptorCount = S;
        info.dstSet = *dst->data();
        return *this;
      }

      DescriptorWriteInfoWrapper& dstBinding(uint32_t binding) {
        info.dstBinding = binding;
        return *this;
      }

      DescriptorWriteInfoWrapper& dstArrayElement(uint32_t arrayElement) {
        info.dstArrayElement = arrayElement;
        return *this;
      }

      DescriptorWriteInfoWrapper& descriptorType(VkDescriptorType type) {
        info.descriptorType = type;
        return *this;
      }

      template<template<typename T, typename Allocator = std::allocator<T>> class Container>
      DescriptorWriteInfoWrapper& pImage(const Container<VkDescriptorImageInfo>& pImages) {
        info.pImageInfo = VK_NULL_HANDLE;
        info.pBufferInfo = VK_NULL_HANDLE;
        info.pTexelBufferView = VK_NULL_HANDLE;
        if (!pImage.empty()) {
          info.pImageInfo = pImage.data();
        }
        return *this;
      }

      template<template<typename T, typename Allocator = std::allocator<T>> class Container>
      DescriptorWriteInfoWrapper& pImage(cppvk::DescriptorImageInfoList<Container>&& pImages) {
        this->pImage(std::move(pImages.raw));
        return *this;
      }

      template<template<typename T, typename Allocator = std::allocator<T>> class Container>
      DescriptorWriteInfoWrapper& pBuffer(Container<VkDescriptorBufferInfo>&& pBuffer) {
        info.pImageInfo = VK_NULL_HANDLE;
        info.pBufferInfo = VK_NULL_HANDLE;
        info.pTexelBufferView = VK_NULL_HANDLE;
        if (!pBuffer.empty()) {
          info.pBufferInfo = pBuffer.data();
        }
        return *this;
      }

      template<template<typename T, typename Allocator = std::allocator<T>> class Container>
      DescriptorWriteInfoWrapper& pBuffer(cppvk::DescriptorBufferInfoList<Container>&& pBuffer) {
        this->pBuffer(std::move(pBuffer.raw));
        return *this;
      }

      template<template<typename T, typename Allocator = std::allocator<T>> class Container>
      DescriptorWriteInfoWrapper& pTexelBufferView(const Container<VkBufferView>& pView) {
        info.pImageInfo = VK_NULL_HANDLE;
        info.pBufferInfo = VK_NULL_HANDLE;
        info.pTexelBufferView = VK_NULL_HANDLE;
        if (!pView.empty()) {
          info.pTexelBufferView = pView.data();
        }
        return *this;
      }

  };

  template < template<typename E, typename Allocator = std::allocator<E>>class Container>
  using DescriptorWriteInfoList = cppvk::WrapContainer< VkWriteDescriptorSet, DescriptorWriteInfoWrapper, Container>;

}