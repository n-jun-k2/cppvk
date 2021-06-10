#pragma once

#include "../vk.h"
#include "../type.h"
#include "../pointer.h"
#include "../deleter/deleter.h"


#pragma warning(push)
#pragma warning(disable : 26812)

namespace cppvk {

  class SwapchainBuilder :
  Nondynamicallocation, Noncopyable {
  private:

      VkSwapchainCreateInfoKHR m_info;
      cppvk::DeviceRef m_refDevice;
      cppvk::SurfaceRef m_refSurface;
      cppvk::SwapchainRef m_refSwapchain;
      cppvk::Indexs tempQueueFamilyIndices;

  public:
    SwapchainBuilder() = delete;
    ~SwapchainBuilder() = default;


    explicit SwapchainBuilder(cppvk::DeviceRef refDevice)
    : m_refDevice(refDevice) {
      m_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
      m_info.pNext = VK_NULL_HANDLE;
      m_info.flags = 0;
      m_info.surface = VK_NULL_HANDLE;
      m_info.oldSwapchain = VK_NULL_HANDLE;
    }

    cppvk::SwapchainPtr create(AllocationCallbacksPtr callbacks = nullptr) {
      auto pDevice = m_refDevice.lock();
      auto pSurface = m_refSurface.lock();
      if (pDevice && pSurface) {
        m_info.surface = pSurface.get();
        auto pSwapchain = m_refSwapchain.lock();
        if (pSwapchain) m_info.oldSwapchain = pSwapchain.get();
        VkSwapchainKHR vkSwapchain;
        checkVk(vkCreateSwapchainKHR(pDevice.get(), &m_info, callbacks ? callbacks.get() : VK_NULL_HANDLE, &vkSwapchain));
        return SwapchainPtr(vkSwapchain, SwapchainDeleter(pDevice, callbacks));
      }
      throw std::runtime_error("Vulkan context does not exist");
    }

    SwapchainBuilder& surface(SurfaceRef refSurface) {
      m_refSurface = refSurface;
      return *this;
    }

    SwapchainBuilder& oldSwapchain(SwapchainRef refSwapchain){
      m_refSwapchain = refSwapchain;
      return *this;
    }

    SwapchainBuilder& flags(const VkSwapchainCreateFlagsKHR flag) {
      m_info.flags = flag;
      return *this;
    }

    SwapchainBuilder& minImageCount(const uint32_t imageCount) {
      m_info.minImageCount = imageCount;
      return *this;
    }

    SwapchainBuilder& imageFormat(const VkFormat format) {
      m_info.imageFormat = format;
      return *this;
    }

    SwapchainBuilder& imageColorSpace(const VkColorSpaceKHR color) {
      m_info.imageColorSpace = color;
      return *this;
    }

    SwapchainBuilder& imageExtent(VkExtent2D extent) {
      m_info.imageExtent = extent;
      return *this;
    }

    SwapchainBuilder& imageArrayLayers(const uint32_t size) {
      m_info.imageArrayLayers = size;
      return *this;
    }

    SwapchainBuilder& imageUsage(const VkImageUsageFlags flag) {
      m_info.imageUsage = flag;
      return *this;
    }

    template<template<typename T, class Allocate = std::allocator<T>>class Container>
    SwapchainBuilder& queueFamilyIndices(const Container<uint32_t>& pQueueFamilyIndices) {
      m_info.queueFamilyIndexCount = static_cast<uint32_t>(pQueueFamilyIndices.size());
      m_info.pQueueFamilyIndices = nullptr;
      if (m_info.queueFamilyIndexCount > 0)
        m_info.pQueueFamilyIndices = pQueueFamilyIndices.data();
      m_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
      if (m_info.queueFamilyIndexCount > 1)
        m_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
      return *this;
    }

    SwapchainBuilder& preTransform(const VkSurfaceTransformFlagBitsKHR flags) {
      m_info.preTransform = flags;
      return *this;
    }

    SwapchainBuilder& compositeAlpha(const VkCompositeAlphaFlagBitsKHR alpha) {
      m_info.compositeAlpha = alpha;
      return *this;
    }

    SwapchainBuilder& presentMode(const VkPresentModeKHR present) {
      m_info.presentMode = present;
      return *this;
    }

    SwapchainBuilder& clippedOn() {
      m_info.clipped = VK_TRUE;
      return *this;
    }

    SwapchainBuilder& clippedOff() {
      m_info.clipped = VK_FALSE;
      return *this;
    }
  };
}

#pragma warning(pop)