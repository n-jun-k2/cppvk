#pragma once

#include "../vk.h"
#include "../type.h"

namespace cppvk {

  class DescriptorSetLayoutInfoWrapper {
    private:
      VkDescriptorSetLayoutBinding m_info;
    public:
      DescriptorSetLayoutInfoWrapper() {
        m_info.pImmutableSamplers = nullptr;
      }
      DescriptorSetLayoutInfoWrapper& binding(const int binding) {
        m_info.binding = binding;
        return *this;
      }
      DescriptorSetLayoutInfoWrapper& type(const VkDescriptorType type) {
        m_info.descriptorType = type;
        return *this;
      }
      DescriptorSetLayoutInfoWrapper& count(const uint32_t count) {
        m_info.descriptorCount = count;
        return *this;
      }
      DescriptorSetLayoutInfoWrapper& stageFlags(const VkShaderStageFlags flags) {
        m_info.stageFlags = flags;
        return *this;
      }
      template<template< class T, class Allocator = std::allocator<T>> class Container>
      DescriptorSetLayoutInfoWrapper& immutableSamplers(Container<VkSampler>& list) {
        m_info.pImmutableSamplers = nullptr;
        if (!list.empty())
          m_info.pImmutableSamplers = list.data();
        return *this;
      }
  };

  template < template<typename E, typename Allocator = std::allocator<E>>class Container>
  using DescriptorSetLayoutBindingList = cppvk::WrapContainer< VkDescriptorSetLayoutBinding, DescriptorSetLayoutInfoWrapper, Container>;

}