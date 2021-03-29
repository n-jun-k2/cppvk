#pragma once

#include "../vk.h"
#include "../type.h"
#include "../pointer.h"
#include "../deleter/deleter.h"


namespace cppvk {
  class ImageViewBuilder : Noncopyable, Nondynamicallocation {
    private:
      VkImageViewCreateInfo m_info;
      cppvk::DeviceRef m_refDevice;
      cppvk::ImageRef m_refImage;
    public:
      explicit ImageViewBuilder(DeviceRef pLogicalDevice)
      : m_refDevice(pLogicalDevice) {
        m_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        m_info.pNext = VK_NULL_HANDLE;
        m_info.flags = 0;
      }
      ~ImageViewBuilder() = default;

      cppvk::ImageViewPtr create(AllocationCallbacksPtr callbacks = nullptr) {
        auto pImage = m_refImage.lock();
        auto pLogicalDevice = m_refDevice.lock();
        if (pLogicalDevice || pImage) {

          if(pImage)
            m_info.image = pImage.get();

          VkImageView imageView;
          checkVk(vkCreateImageView(pLogicalDevice.get(), &m_info, callbacks ? callbacks.get() : VK_NULL_HANDLE, &imageView));
          return ImageViewPtr(imageView, ImageViewDeleter(pLogicalDevice, callbacks));
        }
        throw std::runtime_error("Failed to create ImageView");
      }

      ImageViewBuilder& flag(const VkImageViewCreateFlags& flags) {
        m_info.flags = flags;
        return *this;
      }

      ImageViewBuilder& format(VkFormat format) {
        m_info.format = format;
        return *this;
      }

      ImageViewBuilder& components(const VkComponentMapping& mapping) {
        m_info.components = mapping;
        return *this;
      }

      ImageViewBuilder& viewType(VkImageViewType type) {
        m_info.viewType = type;
        return *this;
      }

      ImageViewBuilder& subresourceRange(const VkImageSubresourceRange& range) {
        m_info.subresourceRange = range;
        return *this;
      }

      ImageViewBuilder& image(ImageRef refImage) {
        m_refImage = refImage;
        return *this;
      }

      ImageViewBuilder& image(VkImage image) {
        m_info.image = image;
        return *this;
      }
  };
}