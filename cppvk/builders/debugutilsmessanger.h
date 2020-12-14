#pragma once

#include "../vk.h"
#include "../context.h"
#include "builder.h"
#include "../objects/debugutilsmessanger.h"
#include <stdexcept>

namespace cppvk {

  /// <summary>
  /// 
  /// </summary>
  class DebugUtilsMessengerBuilder : public Builder{

  private:

    VkDebugUtilsMessengerCreateInfoEXT info;

    /// <summary>
    /// 
    /// </summary>
    /// <param name="instance"></param>
    /// <param name="pInfo"></param>
    /// <param name="pAllocator"></param>
    /// <param name="pDebugMessenger"></param>
    /// <returns></returns>
    static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pInfo,
      const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
    {
      auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
      if (func != nullptr)return func(instance, pInfo, pAllocator, pDebugMessenger);
      return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    /// <summary>
    /// 
    /// </summary>
    /// <param name="instance"></param>
    /// <param name="debugMessenger"></param>
    /// <param name="pAllocator"></param>
    static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
    {
      auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
      if (func != nullptr)func(instance, debugMessenger, pAllocator);
    }

    /// <summary>
    /// 
    /// </summary>
    /// <param name=""></param>
    /// <returns></returns>
    virtual cppvk::DebugUtilsMessenger* createimpl(const VkAllocationCallbacks* arg) override {

      if (auto ctx = this->context.lock()) {

        checkVk(CreateDebugUtilsMessengerEXT(ctx->instance, &info, arg, &ctx->messager));

        return new DebugUtilsMessenger(ctx, [=](cppvk::Context& p) {
          std::cout << "vkDestroyDebugUtilsMessengerEXT" << std::endl;
          DestroyDebugUtilsMessengerEXT(p.instance, p.messager, arg);
        });

      }

      throw std::runtime_error("Vulkan context does not exist");

    }


  public:


    DebugUtilsMessengerBuilder() = delete;
    DebugUtilsMessengerBuilder(const DebugUtilsMessengerBuilder&) = default;
    DebugUtilsMessengerBuilder& operator=(const DebugUtilsMessengerBuilder&) = default;
    DebugUtilsMessengerBuilder(DebugUtilsMessengerBuilder&&) = default;
    DebugUtilsMessengerBuilder& operator=(DebugUtilsMessengerBuilder&&) = default;

    /// <summary>
    /// 
    /// </summary>
    /// <param name="ctx"></param>
    explicit DebugUtilsMessengerBuilder(cppvk::Context::Ptr ctx) : cppvk::Builder(ctx) {
      info = {};
      info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
      info.pNext = NULL;
      info.flags = 0;
      info.pUserData = nullptr;
    }
    ~DebugUtilsMessengerBuilder() {}


    /// <summary>
    /// 
    /// </summary>
    /// <param name="value"></param>
    /// <returns></returns>
    DebugUtilsMessengerBuilder severity(VkDebugUtilsMessageSeverityFlagsEXT value) {
      info.messageSeverity = value;
      return *this;
    }

    /// <summary>
    /// 
    /// </summary>
    /// <param name="value"></param>
    /// <returns></returns>
    DebugUtilsMessengerBuilder type(VkDebugUtilsMessageTypeFlagsEXT value) {
      info.messageType = value;
      return *this;
    }

    /// <summary>
    /// 
    /// </summary>
    /// <param name="value"></param>
    /// <returns></returns>
    DebugUtilsMessengerBuilder callback(PFN_vkDebugUtilsMessengerCallbackEXT value) {
      info.pfnUserCallback = value;
      return *this;
    }

    /// <summary>
    /// 
    /// </summary>
    /// <param name="callbacks"></param>
    /// <returns></returns>
    DebugUtilsMessenger::Ptr create(const VkAllocationCallbacks* callbacks = VK_NULL_HANDLE) {
      return DebugUtilsMessenger::Ptr(this->createimpl(callbacks));
    }

  };//DebugMessengerPtr
}