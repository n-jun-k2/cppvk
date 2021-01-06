#pragma once

#include "../vk.h"
#include "../objects/object.h"
#include "../objects/instance.h"
#include  "../objects/surface.h"

#include "physicaldevicedetails.h"
#include "surfacedetails.h"

namespace cppvk {

  class cppvk::Instance::Surface::PhysicalDevice;
  using ChoosePhysicalDeviceFunc = std::function<bool(typename cppvk::Instance::Surface::PhysicalDevice&)>;

  class Surface::PhysicalDevice {

    public:
      PhysicalDevice() = delete;
      ~PhysicalDevice() = default;
      PhysicalDevice(const PhysicalDevice&) = default;
      PhysicalDevice& operator=(const PhysicalDevice&) = default;
      PhysicalDevice(PhysicalDevice&&) = default;
      PhysicalDevice& operator=(PhysicalDevice&&)=default;

      using pointer = std::shared_ptr<Surface::PhysicalDevice>;
      using reference = std::weak_ptr<Surface::PhysicalDevice>;

      class LogicalDevice;
      const PhysicalDeviceDetails details;

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


      PhysicalDeviceSurfaceDetails getSurfaceDetails(cppvk::Surface::reference refSurface) const {
        if (auto pSurface = refSurface.lock()) {
          return PhysicalDeviceSurfaceDetails(physicalDevice, pSurface->surface);
        }
        throw std::runtime_error("Surface does not exist");
      }

    private:
      VkPhysicalDevice physicalDevice;

      PhysicalDevice(VkPhysicalDevice pDevice) : physicalDevice(pDevice), details(pDevice) { }
  };

  using PhysicalDevice = Surface::PhysicalDevice;
}