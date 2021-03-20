#pragma once

#include "vk.h"

#include <array>
#include <memory>

namespace cppvk {

  template<class T>
  using pointer = std::shared_ptr<std::remove_pointer_t<T>>;

  template<class T>
  using reference = std::weak_ptr<std::remove_pointer_t<T>>;

  using InstancePtr = pointer<VkInstance>;
  using InstanceRef = reference<VkInstance>;

  using DebugUtilsMessengerPtr = pointer<VkDebugUtilsMessengerEXT>;
  using DebugUtilsMessengerRef = reference<VkDebugUtilsMessengerEXT>;

  using DevicePtr = pointer<VkDevice>;
  using DeviceRef = reference<VkDevice>;

  using SurfacePtr = pointer<VkSurfaceKHR>;
  using SurfaceRef = reference<VkSurfaceKHR>;

  using CommandPoolPtr = pointer<VkCommandPool>;
  using CommandPoolRef = reference<VkCommandPool>;

  template<size_t Length>
  using CommandBufferPtr = pointer<std::array<VkCommandBuffer, Length>>;
  template<size_t Length>
  using CommandBufferRef = reference<std::array<VkCommandBuffer, Length>>;

  using SwapchainPtr = pointer<VkSwapchainKHR>;
  using SwapchainRef = reference<VkSwapchainKHR>;

  using ImagePtr = pointer<VkImage>;
  using ImageRef = reference<VkImage>;

  using AllocationCallbacksPtr = pointer<VkAllocationCallbacks>;
  using AllocationCallbacksRef = reference<VkAllocationCallbacks>;
}