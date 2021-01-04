#pragma once

#include "vk.h"
#include "objects/object.h"
#include "objects/instance.h"

namespace cppvk {

   class cppvk::Instance::PhysicalDevice;
   using ChoosePhysicalDeviceFunc = std::function<bool(typename cppvk::Instance::PhysicalDevice&)>;

  class Instance::PhysicalDevice {

    public:
      PhysicalDevice() = delete;
      ~PhysicalDevice() = default;
      PhysicalDevice(const PhysicalDevice&) = default;
      PhysicalDevice& operator=(const PhysicalDevice&) = default;
      PhysicalDevice(PhysicalDevice&&) = default;
      PhysicalDevice& operator=(PhysicalDevice&&)=default;

      using pointer = std::shared_ptr<Instance::PhysicalDevice>;
      using reference = std::weak_ptr<Instance::PhysicalDevice>;

      class LogicalDevice;

      static PhysicalDevice choosePhysicalDevice(Instance::reference refInstance, ChoosePhysicalDeviceFunc isSuitable) {
          if (auto pInstance = refInstance.lock())
          {
              const auto gpus = pInstance->getEnumeratePhysicalDevices();
              if (gpus.empty())
                  throw std::runtime_error("failed to find GPUs with vulkan support!");

              for (auto&& gpu : gpus) {
                  auto dev_set = PhysicalDevice(gpu);

                  if (!isSuitable(dev_set))	continue;

                  return dev_set;
              }

              throw std::runtime_error("There is no GPU that meets the conditions");
          }
          throw std::runtime_error("Vulkan context does not exist");
      }

      VkPhysicalDeviceProperties const& getProperties() const noexcept {
          return properties;
      }
      VkPhysicalDeviceFeatures const& getFeatures() const noexcept {
          return features;
      }
      VkPhysicalDeviceMemoryProperties const& getMemoryProperties() const noexcept {
          return memoryProperties;
      }
      ExtensionPropertiesList const& getExtensionList() const noexcept {
          return extensions;
      }
      LayerPropertiesList const& getLayerPropertieList() const noexcept {
          return validations;
      }
      PhysicalDeviceQueueProps const& getQueuePropertieList() const noexcept {
          return queueProperties;
      }

    private:
      VkPhysicalDevice physicalDevice;

      VkPhysicalDeviceProperties properties;
      VkPhysicalDeviceFeatures features;
      VkPhysicalDeviceMemoryProperties memoryProperties;
      ExtensionPropertiesList extensions;
      LayerPropertiesList validations;
      PhysicalDeviceQueueProps queueProperties;

      PhysicalDevice(VkPhysicalDevice pDevice) : physicalDevice(pDevice) {
          vkGetPhysicalDeviceProperties(physicalDevice, &properties);
          vkGetPhysicalDeviceFeatures(physicalDevice, &features);
          vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);
          getEnumeratePhysicalDeviceQueueProps();
          getEnumeratePhysicalDeviceExtensions();
          getEnumerateDeviceLayer();
      }

      void getEnumeratePhysicalDeviceQueueProps() {
          uint32_t  count = 0;
          vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, nullptr);
          queueProperties.resize(count);
          vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, queueProperties.data());
      }

      void getEnumeratePhysicalDeviceExtensions() {
          uint32_t size = 0;
          checkVk(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &size, nullptr));
          extensions.resize(size);
          checkVk(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &size, extensions.data()));
      }

      void getEnumerateDeviceLayer() {
          uint32_t size = 0;
          checkVk(vkEnumerateDeviceLayerProperties(physicalDevice, &size, nullptr));
          validations.resize(size);
          checkVk(vkEnumerateDeviceLayerProperties(physicalDevice, &size, validations.data()));
      }

  };

  using PhysicalDevice = Instance::PhysicalDevice;
}