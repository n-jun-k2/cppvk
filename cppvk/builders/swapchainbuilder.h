#pragma once

#include "../vk.h"
#include "../objects/logicaldevice.h"
#include "../objects/swapchain.h"
#include "../objects/surface.h"
#include "Ibuilder.h"


#pragma warning(push)
#pragma warning(disable : 26812)

namespace cppvk {

  class PreSwapchainBuilder {
    private:
      VkSwapchainCreateInfoKHR info;
      cppvk::LogicalDevice::reference object;

      virtual cppvk::Swapchain* createimpl(const VkAllocationCallbacks* ) = 0;

    public:

      PreSwapchainBuilder(cppvk::Surface::pointer surface, cppvk::LogicalDevice::pointer pDevice)
      : cppvk::IBuilder(), object(pDevice) {
        info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        info.pNext = VK_NULL_HANDLE;
        info.flags = 0;
        info.surface = surface->surface;
      }

      Swapchain::pointer create(const VkAllocationCallbacks* callbacks = VK_NULL_HANDLE) {
        return Swapchain::pointer(this->createimpl(callbacks));
      }

      PreSwapchainBuilder() = delete;
      PreSwapchainBuilder(const PreSwapchainBuilder&) = default;
      PreSwapchainBuilder& operator=(const PreSwapchainBuilder&) = default;
      PreSwapchainBuilder(PreSwapchainBuilder&&) = default;
      PreSwapchainBuilder& operator=(PreSwapchainBuilder&&) = default;
      ~PreSwapchainBuilder() = default;

  };

  class Swapchain::SwapchainBuilder : public Surface::PreSwapchainBuilder {

  private:
      cppvk::Indexs tempQueueFamilyIndices;

      virtual cppvk::Swapchain* createimpl(const VkAllocationCallbacks* arg) override {
        if (auto pDevice = object.lock()) {
          auto pSwapchain = new cppvk::Swapchain(pDevice);
          auto& vkDevice = pDevice->device;
          auto& vkSwapchain = pSwapchain->swapchain;
          checkVk(vkCreateSwapchainKHR(vkDevice, &this->info, arg, &vkSwapchain));

          *(pDevice->destroy) += [=]() {
            std::cout << "vkDestorySwapchain : " << vkSwapchain << std::endl;
            vkDestroySwapchainKHR(vkDevice, vkSwapchain, arg);
          };
          return pSwapchain;
        }
        throw std::runtime_error("Vulkan context does not exist");
      }
  public:

    SwapchainBuilder() = delete;
    SwapchainBuilder(const SwapchainBuilder&) = default;
    SwapchainBuilder& operator=(const SwapchainBuilder&) = default;
    SwapchainBuilder(SwapchainBuilder&&) = default;
    SwapchainBuilder& operator=(SwapchainBuilder&&) = default;
    ~SwapchainBuilder() = default;

    SwapchainBuilder(cppvk::Surface::pointer pSurface, cppvk::LogicalDevice::pointer pDevice,  Swapchain::pointer pSwapchain = nullptr)
    : cppvk::Surface::PreSwapchainBuilder(pSurface, pDevice) {
      if (pSwapchain)
        info.oldSwapchain = pSwapchain->swapchain;
      else
        info.oldSwapchain = VK_NULL_HANDLE;
    }

    SwapchainBuilder& flags(const VkSwapchainCreateFlagsKHR flag) {
      info.flags = flag;
      return *this;
    }

    SwapchainBuilder& minImageCount(const uint32_t imageCount) {
      info.minImageCount = imageCount;
      return *this;
    }

    SwapchainBuilder& imageFormat(const VkFormat format) {
      info.imageFormat = format;
      return *this;
    }

    SwapchainBuilder& imageColorSpace(const VkColorSpaceKHR color) {
      info.imageColorSpace = color;
      return *this;
    }

    SwapchainBuilder& imageExtent(VkExtent2D extent) {
      info.imageExtent = extent;
      return *this;
    }

    SwapchainBuilder& imageArrayLayers(const uint32_t size) {
      info.imageArrayLayers = size;
      return *this;
    }

    SwapchainBuilder& imageUsage(const VkImageUsageFlags flag) {
      info.imageUsage = flag;
      return *this;
    }

    SwapchainBuilder& imageSharingMode(const VkSharingMode mode) {
      info.imageSharingMode = mode;
      return *this;
    }

    SwapchainBuilder& queueFamilyIndices(const cppvk::Indexs& pQueueFamilyIndices) {
      info.queueFamilyIndexCount = static_cast<uint32_t>(pQueueFamilyIndices.size());
      info.pQueueFamilyIndices = pQueueFamilyIndices.data();
      return *this;
    }

    SwapchainBuilder& queueFamilyIndices(cppvk::Indexs&& pQueueFamilyIndices) {
      tempQueueFamilyIndices = std::move(pQueueFamilyIndices);
      return this->queueFamilyIndices(tempQueueFamilyIndices);
    }

    SwapchainBuilder& preTransform(const VkSurfaceTransformFlagBitsKHR flags) {
      info.preTransform = flags;
      return *this;
    }

    SwapchainBuilder& compositeAlpha(const VkCompositeAlphaFlagBitsKHR alpha) {
      info.compositeAlpha = alpha;
      return *this;
    }

    SwapchainBuilder& presentMode(const VkPresentModeKHR present) {
      info.presentMode = present;
      return *this;
    }

    SwapchainBuilder& clippedOn() {
      info.clipped = VK_TRUE;
      return *this;
    }

    SwapchainBuilder& clippedOff() {
      info.clipped = VK_FALSE;
      return *this;
    }
  };
  using SwapchainBuilder = Swapchain::SwapchainBuilder;
}

#pragma warning(pop)