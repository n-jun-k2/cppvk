#pragma once

#include "../vk.h"
#include "../pointer.h"
#include <array>
#include <memory>

namespace cppvk {

  template<typename T>
  class _deleter {
    public:
      using type = std::remove_pointer_t<T>;
      using pointer = type*;
      _deleter() = delete;
      explicit _deleter(AllocationCallbacksPtr callbacks) : m_callbacks(callbacks) {}
      ~_deleter() = default;
      virtual void operator()(pointer ptr)  = 0;
    protected:
      AllocationCallbacksPtr m_callbacks;
  };

  class InstanceDeleter:public _deleter<VkInstance>{
    public:
      using _deleter::_deleter;
      virtual void operator()(pointer ptr)  override{
        vkDestroyInstance(ptr, m_callbacks ? m_callbacks.get() : VK_NULL_HANDLE);
      }
  };

  class LogicalDeviceDeleter:public _deleter<VkDevice>{
    public:
      using _deleter::_deleter;
      virtual void operator()(pointer ptr)  override{
        vkDestroyDevice(ptr, m_callbacks ? m_callbacks.get() : VK_NULL_HANDLE);
      }
  };

  template<typename T, typename T_BASE>
  class _sub_deleter : public _deleter<T> {
    public:
      using base_type = std::remove_pointer_t<T_BASE>;
      using base_type_pointer = std::shared_ptr<base_type>;
      _sub_deleter() = delete;
      explicit _sub_deleter(base_type_pointer pParent, AllocationCallbacksPtr callbacks)
        : _deleter<T>::_deleter(callbacks), m_pparent(pParent){}
      ~_sub_deleter() = default;
    protected:
      base_type_pointer m_pparent;
  };

  class DebugUtilsMessengerDeleter
    : public _sub_deleter <VkDebugUtilsMessengerEXT, VkInstance> {
    public:
      using _sub_deleter::_sub_deleter;
      virtual void operator()(pointer ptr)  override {
        auto instance = m_pparent.get();
        DestroyDebugUtilsMessengerEXT(instance, ptr, m_callbacks ? m_callbacks.get() : VK_NULL_HANDLE);
      }
  };

  class SurfacerDeleter
    : public _sub_deleter <VkSurfaceKHR, VkInstance> {
  public:
    using _sub_deleter::_sub_deleter;
    virtual void operator()(pointer ptr)  override {
      auto instance = m_pparent.get();
      vkDestroySurfaceKHR(instance, ptr, m_callbacks ? m_callbacks.get() : VK_NULL_HANDLE);
    }
  };

  class CommandPoolDeleter
    : public _sub_deleter <VkCommandPool, VkDevice> {
  public:
    using _sub_deleter::_sub_deleter;
    virtual void operator()(pointer ptr) override {
      auto device = m_pparent.get();
      vkDestroyCommandPool(device, ptr, m_callbacks ? m_callbacks.get() : VK_NULL_HANDLE);
    }
  };

  class SwapchainDeleter
    : public _sub_deleter <VkSwapchainKHR, VkDevice> {
  public:
    using _sub_deleter::_sub_deleter;
    virtual void operator()(pointer ptr) override {
      auto device = m_pparent.get();
      vkDestroySwapchainKHR(device, ptr, m_callbacks ? m_callbacks.get() : VK_NULL_HANDLE);
    }
  };

  class ImageDeleter : public _sub_deleter<VkImage, VkDevice> {
    public:
    using _sub_deleter::_sub_deleter;
    virtual void operator()(pointer ptr) override {
      auto device = m_pparent.get();
      vkDestroyImage(device, ptr, m_callbacks ? m_callbacks.get() : VK_NULL_HANDLE);
    }
  };


  using deivce_and_commandpool = std::pair< pointer<VkDevice_T>, pointer<VkCommandPool_T>>;

  template< std::size_t L>
  using _cmd_sub_deleter = _sub_deleter <std::array<VkCommandBuffer, L>, deivce_and_commandpool>;

  template< std::size_t  _L>
  class CommandBufferDeleter : public   _cmd_sub_deleter<_L> {
  public:
    using _cmd_sub_deleter<_L>::_cmd_sub_deleter;
    virtual void operator()(typename  _cmd_sub_deleter<_L>::pointer ptr) override {
      auto device = this->m_pparent->first.get();
      auto cmdPool = this->m_pparent->second.get();
      vkFreeCommandBuffers(device, cmdPool, _L, ptr->data());
    }
  };

}