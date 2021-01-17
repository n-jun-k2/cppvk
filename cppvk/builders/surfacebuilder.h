#pragma once

#include "../vk.h"
#include "../objects/object.h"
#include "../objects/instance.h"
#include "../objects/surface.h"
#include "Ibuilder.h"

namespace cppvk {

  class Surface::SurfaceBuilder : public IBuilder {
  private:
    VkWin32SurfaceCreateInfoKHR  info;
    Instance::reference object;

    virtual cppvk::Surface* createimpl(const VkAllocationCallbacks* arg) override {

      if (auto pInstance = this->object.lock())
      {
        auto pSurface = new cppvk::Surface(pInstance);
        auto& vkInstance = pInstance->instance;
        auto& vkSurface = pSurface->surface;
        checkVk(vkCreateWin32SurfaceKHR(vkInstance, &info, arg, &vkSurface));

        *(pInstance->destroy) += [=]() {
          std::cout << "vkDestroySurfaceKHR :" << vkSurface << std::endl;
          vkDestroySurfaceKHR(vkInstance, vkSurface, arg);
        };
        return pSurface;
      }

      throw std::runtime_error("Vulkan context does not exist");
    }

  public:

    SurfaceBuilder() = delete;
    SurfaceBuilder(const SurfaceBuilder&) = default;
    SurfaceBuilder& operator=(const SurfaceBuilder&) = default;
    SurfaceBuilder(SurfaceBuilder&&) = default;
    SurfaceBuilder& operator=(SurfaceBuilder&&) = default;
    ~SurfaceBuilder() = default;

    explicit SurfaceBuilder(cppvk::Instance::reference refInstance) : object(refInstance) {
      info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
      info.hinstance = GetModuleHandle(nullptr);
      info.pNext = NULL;
      info.flags = 0;
    }

    /// <summary>
    ///
    /// </summary>
    /// <param name="item"></param>
    /// <returns></returns>
    SurfaceBuilder& hwnd(HWND item) {
      info.hwnd = item;
      return *this;
    }

    /// <summary>
    ///
    /// </summary>
    /// <param name="item"></param>
    /// <returns></returns>
    SurfaceBuilder& hinstance(HINSTANCE item) {
      info.hinstance = item;
      return *this;
    }

    /// <summary>
    /// 
    /// </summary>
    /// <param name="callbacks"></param>
    /// <returns></returns>
    typename cppvk::Instance::Surface::pointer create(const VkAllocationCallbacks* callbacks = VK_NULL_HANDLE){
      return Surface::pointer(this->createimpl(callbacks));
    }

  };

  using SurfaceBuilder = Surface::SurfaceBuilder;
}