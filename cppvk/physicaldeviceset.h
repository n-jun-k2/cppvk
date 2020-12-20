#pragma once
#include "vk.h"
#include <cassert>
#include <functional>

namespace cppvk {

  class PhysicalDeviceSet;

  /// <summary>
  ///
  /// </summary>
  using ChoosePhysicalDeviceFuncInstanceBuilder = std::function<bool(PhysicalDeviceSet&)>;

  /// <summary>
  ///
  /// </summary>
  class PhysicalDeviceSet {

  private:

    VkPhysicalDevice device;

    VkPhysicalDeviceProperties props;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties memory;
    ExtensionPropertiesList extensions;
    LayerPropertiesList validations;
    PhysicalDeviceQueueProps qprops;

    PhysicalDeviceQueueProps getEnumeratePhysicalDeviceQueueProps(const VkPhysicalDevice gpu) {
      assert(gpu != VK_NULL_HANDLE);
      uint32_t  count = 0;
      vkGetPhysicalDeviceQueueFamilyProperties(gpu, &count, nullptr);
      PhysicalDeviceQueueProps queue(count);
      vkGetPhysicalDeviceQueueFamilyProperties(gpu, &count, queue.data());
      return queue;
    }

    ExtensionPropertiesList getEnumeratePhysicalDeviceExtensions(const VkPhysicalDevice gpu) {
      assert(gpu != VK_NULL_HANDLE);
      uint32_t size = 0;
      checkVk(vkEnumerateDeviceExtensionProperties(gpu, nullptr, &size, nullptr));
      ExtensionPropertiesList list(size);
      checkVk(vkEnumerateDeviceExtensionProperties(gpu, nullptr, &size, list.data()));
      return list;
    }

    LayerPropertiesList getEnumerateDeviceLayer(const VkPhysicalDevice gpu) {
      assert(gpu != VK_NULL_HANDLE);
      uint32_t size = 0;
      checkVk(vkEnumerateDeviceLayerProperties(gpu, &size, nullptr));
      LayerPropertiesList list(size);
      checkVk(vkEnumerateDeviceLayerProperties(gpu, &size, list.data()));
      return list;
    }

  public:

    /// <summary>
    ///
    /// </summary>
    /// <param name="gpu"></param>
    PhysicalDeviceSet(VkPhysicalDevice gpu) :device(gpu) {
      vkGetPhysicalDeviceProperties(gpu, &props);
      vkGetPhysicalDeviceFeatures(gpu, &features);
      vkGetPhysicalDeviceMemoryProperties(gpu, &memory);
      extensions = getEnumeratePhysicalDeviceExtensions(gpu);
      validations = getEnumerateDeviceLayer(gpu);
      qprops = getEnumeratePhysicalDeviceQueueProps(gpu);
    }

    /// <summary>
    ///
    /// </summary>
    /// <returns></returns>
    VkPhysicalDeviceProperties const& getProps() const {
      return this->props;
    }

    /// <summary>
    ///
    /// </summary>
    /// <returns></returns>
    VkPhysicalDeviceFeatures const& getFeatures() const {
      return this->features;
    }

    /// <summary>
    ///
    /// </summary>
    /// <returns></returns>
    VkPhysicalDeviceMemoryProperties const& getMemory() const {
      return this->memory;
    }

    /// <summary>
    ///
    /// </summary>
    /// <returns></returns>
    ExtensionPropertiesList const& getExtensions() const {
      return this->extensions;
    }

    /// <summary>
    ///
    /// </summary>
    /// <returns></returns>
    LayerPropertiesList const& getValidations() const {
      return this->validations;
    }

    /// <summary>
    ///
    /// </summary>
    /// <returns></returns>
    PhysicalDeviceQueueProps const& getQprops() const {
      return this->qprops;
    }

    /// <summary>
    ///
    /// </summary>
    /// <param name="pDevice"></param>
    /// <param name="pConfiginfo"></param>
    /// <param name="pAllocation"></param>
    void CreateDevice(VkDevice *pDevice, const VkDeviceCreateInfo* pConfiginfo, const VkAllocationCallbacks* pAllocation) {
      auto err = vkCreateDevice(this->device, pConfiginfo, pAllocation, pDevice);
      checkVk(err);
    }

  };

}