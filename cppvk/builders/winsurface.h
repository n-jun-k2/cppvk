#pragma once

#include "../vk.h"
#include "../context.h"
#include "builder.h"
#include "../objects/winsurface.h"

#include <stdexcept>

namespace cppvk {

  /// <summary>
  /// 
  /// </summary>
  class WinSurfaceBuilder : public Builder {

    private:
      VkWin32SurfaceCreateInfoKHR info = {};

      /// <summary>
      /// 
      /// </summary>
      virtual WinSurface* createimpl(const VkAllocationCallbacks* arg = VK_NULL_HANDLE) override {

        if (auto ctx = this->context.lock()) {

          cppvk::checkVk(vkCreateWin32SurfaceKHR(ctx->instance, &info, arg, &ctx->surface));

          return new WinSurface(ctx, [=](cppvk::Context& p) {
              std::cout << "vkDestroySurfaceKHR" << std::endl;
              vkDestroySurfaceKHR(p.instance, p.surface, arg);
            });

        }

        throw std::runtime_error("Vulkan context does not exist");

      }

    public:

      /// <summary>
      ///  Construct a new Win Surface Builder object
      /// </summary>
      /// <param name="ctx"></param>
      WinSurfaceBuilder(cppvk::Context::Ptr ctx) : Builder(ctx){
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
      WinSurfaceBuilder hwnd(HWND item) {
        info.hwnd = item;
		  	return *this;
      }

		  /// <summary>
		  /// 
		  /// </summary>
		  /// <param name="item"></param>
		  /// <returns></returns>
      WinSurfaceBuilder hinstance(HINSTANCE item) {
        info.hinstance = item;
		  	return *this;
      }

		  /// <summary>
		  /// 
		  /// </summary>
		  /// <returns></returns>
      WinSurface::Ptr create(const VkAllocationCallbacks* callbacks = VK_NULL_HANDLE) {
        return WinSurface::Ptr(this->createimpl(callbacks));
      }

  };

}