#pragma once

#include "../vk.h"
#include "../type.h"
#include "../pointer.h"
#include "../deleter/deleter.h"

namespace cppvk {
  class ShaderModuleBuilder : Noncopyable, Nondynamicallocation {
    private:
      VkShaderModuleCreateInfo m_info;
      cppvk::DeviceRef m_refDevice;

    public:
      explicit ShaderModuleBuilder(DeviceRef pLogicalDevice) : m_refDevice(pLogicalDevice) {
        m_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        m_info.pNext = VK_NULL_HANDLE;
        m_info.pCode = VK_NULL_HANDLE;
        m_info.codeSize = 0;
        m_info.flags = 0;
      }
      ~ShaderModuleBuilder() = default;

      ShaderModulePtr create(AllocationCallbacksPtr callbacks) {
        if(auto pDevice = m_refDevice.lock()) {
          VkShaderModule shaderModule;
          checkVk(vkCreateShaderModule(pDevice.get(), &m_info, callbacks ? callbacks.get() : VK_NULL_HANDLE, &shaderModule));
          return ShaderModulePtr(shaderModule, ShaderModuleDeleter(pDevice, callbacks));
        }
        throw std::runtime_error("Faild to create ShadeModule");
      }

      ShaderModuleBuilder& flags(VkShaderModuleCreateFlags flags) {
        m_info.flags = flags;
        return *this;
      }

      template<template<typename E, typename Allocator=std::allocator<E>>typename Container>
      ShaderModuleBuilder& code(const Container<char>& pCode) {
        m_info.codeSize = static_cast<uint32_t>(pCode.size());
        m_info.pCode = reinterpret_cast<const uint32_t*>(code.data());
        return *this;
      }

  };
}