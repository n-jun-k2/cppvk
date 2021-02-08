#pragma once

#include "../vk.h"
#include "../objects/image.h"
#include "Ibuilder.h"

namespace cppvk {
  class Image::ImageBuilder : public cppvk::IBuilder {
    private:
      VkImageCreateInfo info;
      cppvk::LogicalDevice::reference refLogicalDevice;

      virtual cppvk::Image* createimpl(const VkAllocationCallbacks* arg) override {
        if(auto pLogicalDevice = refLogicalDevice.lock()){
          auto pImage = new Image(pLogicalDevice);
          auto& vkDevice = pLogicalDevice->device;
          auto& vkImage = pImage->image;

          vkCreateImage(vkDevice, &info, arg, &vkImage);

          *(pLogicalDevice->destroy) += [=]() {
            std::cout << "vkDestroyImage : " << vkImage << std::endl;
            vkDestroyImage(vkDevice, vkImage, arg);
          };
          return pImage;

        }
        throw std::runtime_error("Vulkan context does not exist");
      }

    public:
      explicit ImageBuilder(cppvk::LogicalDevice::reference pLogicalDevice)
      : refLogicalDevice(pLogicalDevice){
        info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        info.pNext = VK_NULL_HANDLE;
        info.flags = 0;
        info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        info.tiling = VK_IMAGE_TILING_OPTIMAL
      }

      cppvk::Image::pointer create(const VkAllocationCallbacks* callbacks = VK_NULL_HANDLE) {
        return Image::pointer(this->createimpl(callbacks));
      }

      ImageBuilder() = delete;
      ImageBuilder(const ImageBuilder&) = default;
      ImageBuilder& operator=(const ImageBuilder&) = default;
      ImageBuilder(ImageBuilder&&) = default;
      ImageBuilder& operator=(ImageBuilder&&) = default;

      ImageBuilder& pNext() {
        info.pNext = VK_NULL_HANDLE;
        return *this;
      }

      ImageBuilder& flags(VkImageCreateFlags flag){
        info.flags = flag;
        return *this;
      }

      ImageBuilder& imageType(VkImageType type) {
        info.imageType = type;
        return *this;
      }

      ImageBuilder& format(VkFormat formats) {
        info.format = formats;
        return *this;
      }

      ImageBuilder& extent(VkExtent3D extents) {
        info.extent = extents;
        return *this;
      }

      ImageBuilder& mipLevels(const uint32_t level) {
        info.mipLevels = level;
        return *this;
      }

      ImageBuilder& arrayLayers(const uint32_t layers) {
        info.arrayLayers = layers;
        return *this;
      }

      ImageBuilder& samples(VkSampleCountFlagBits bits) {
        info.samples = bits;
        return *this;
      }

      ImageBuilder& tiling(VkImageTiling til) {
        info.tiling = til;
        return *this;
      }

      ImageBuilder& usage(VkImageUsageFlags usageFlags) {
        info.usage = usageFlags;
        return *this;
      }

      ImageBuilder& sharingMode(VkSharingMode mode) {
        info.sharingMode = mode;
        return *this;
      }

      ImageBuilder& queueFamilyIndices(cppvk::Indexs& indices) {
        info.queueFamilyIndexCount = static_cast<uint32_t>(indices.size());
        info.pQueueFamilyIndices = indices.data();
        return *this;
      }

      ImageBuilder& initialLayout(VkImageLayout layout) {
        info.initialLayout = layout;
        return *this;
      }

  };
  using ImageBuilder = cppvk::Image::ImageBuilder;
}