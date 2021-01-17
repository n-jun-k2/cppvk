#pragma once

#include "../vk.h"
#include "../objects/instance.h"
#include "../objects/debugutilsmessenger.h"
#include "Ibuilder.h"

namespace cppvk {
  /// <summary>
  ///
  /// </summary>
  class DebugUtilsMessenger::DebugUtilsMessengerBuilder : public IBuilder{

  private:

    VkDebugUtilsMessengerCreateInfoEXT info;
    cppvk::Instance::reference object;

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
    virtual cppvk::DebugUtilsMessenger* createimpl(const VkAllocationCallbacks*arg ) override {

      if (auto pInstance = this->object.lock()) {

        auto pDebugUtilsMessenger = new DebugUtilsMessenger(pInstance);
        auto& vkInstance = pInstance->instance;
        auto& vkMessenger = pDebugUtilsMessenger->messenger;
        checkVk(CreateDebugUtilsMessengerEXT(vkInstance, &info, arg, &vkMessenger));

        *(pInstance->destroy) += [=]() {
          std::cout << "vkDestroyDebugUtilsMessengerEXT :" << vkMessenger << std::endl;
          DestroyDebugUtilsMessengerEXT(vkInstance, vkMessenger, arg);
        };
        return pDebugUtilsMessenger;
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
    /// <param name="refInstance"></param>
    explicit DebugUtilsMessengerBuilder(cppvk::Instance::reference refInstance) : object(refInstance) {
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
    DebugUtilsMessengerBuilder& severity(VkDebugUtilsMessageSeverityFlagsEXT value) {
      info.messageSeverity = value;
      return *this;
    }

    /// <summary>
    ///
    /// </summary>
    /// <param name="value"></param>
    /// <returns></returns>
    DebugUtilsMessengerBuilder& type(VkDebugUtilsMessageTypeFlagsEXT value) {
      info.messageType = value;
      return *this;
    }

    /// <summary>
    ///
    /// </summary>
    /// <param name="value"></param>
    /// <returns></returns>
    DebugUtilsMessengerBuilder& callback(PFN_vkDebugUtilsMessengerCallbackEXT value) {
      info.pfnUserCallback = value;
      return *this;
    }

    /// <summary>
    ///
    /// </summary>
    /// <param name="callbacks"></param>
    /// <returns></returns>
    DebugUtilsMessenger::pointer create(const VkAllocationCallbacks* callbacks = VK_NULL_HANDLE) {
      return DebugUtilsMessenger::pointer(this->createimpl(callbacks));
    }

  };//DebugUtilsMessengerBuilder

  using DebugUtilsMessengerBuilder = DebugUtilsMessenger::DebugUtilsMessengerBuilder;
}