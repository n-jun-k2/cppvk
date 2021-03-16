#include "Window/App.h"
#include "Window/AppWindow.h"

#include "cppvk/vk.h"
#include "cppvk/pointer.h"
#include "cppvk/builders/instancebuilder.h"
#include "cppvk/builders/debugutilsmessengerbuilder.h"
#include "cppvk/builders/surfacebuilder.h"
#include "cppvk/builders/logicaldevicebuilder.h"
#include "cppvk/builders/commandpoolbuilder.h"
#include "cppvk/info/devicequeueinfo.h"
#include "cppvk/allocator/commandbuffer.h"

#include <algorithm>
#include <set>
#include <vector>

#pragma warning( disable : 4505 4189 26812)

#if _DEBUG
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
  VkDebugUtilsMessageSeverityFlagBitsEXT ,
  VkDebugUtilsMessageTypeFlagsEXT ,
  const VkDebugUtilsMessengerCallbackDataEXT* pcallback,
  void* )
{
  std::cerr << "validation layer : " << pcallback->pMessage << std::endl;
  return VK_FALSE;
}
#endif


/// <summary>
/// Version that returns the index of find_if
/// </summary>
/// <typeparam name="InputIterator"></typeparam>
/// <typeparam name="Predicate"></typeparam>
/// <param name="first"></param>
/// <param name="last"></param>
/// <param name="pred"></param>
/// <returns></returns>
template <class InputIterator, class Predicate>
uint32_t find_if_index(InputIterator first, InputIterator last, Predicate pred) {
  auto itr = std::find_if(first, last, pred);
  if (itr == last)return UINT32_MAX;
  return static_cast<uint32_t>(std::distance(first, itr));
}


class MyContext {

  static constexpr size_t CMD_BUFFER_SIZE = 1;

  cppvk::InstancePtr m_instance;
  cppvk::DebugUtilsMessengerPtr m_debguUtilsMessenger;
  cppvk::SurfacePtr m_surface;
  cppvk::DevicePtr m_logicalDevice;
  cppvk::CommandPoolPtr m_cmdPool;
  cppvk::CommandBufferPtr<CMD_BUFFER_SIZE> m_cmdBuffer;

public:
  MyContext() = default;
  ~MyContext() = default;

  void  WinInit(HWND hwnd, const uint32_t& , const uint32_t& ) {

    auto useDebug = true;
    std::vector<VkExtensionProperties> instanceExtensions;
    cppvk::getEnumerateInstanceExtension(instanceExtensions);
    std::vector<VkLayerProperties> instanceLayer;
    cppvk::getEnumerateInstanceLayer(instanceLayer);

    cppvk::Names extensions{};
    cppvk::Names validationLayers{ "VK_LAYER_KHRONOS_validation" }; // VK_LAYER_LUNARG_standard_validation
    cppvk::Names devExtension = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    for (const auto& e : instanceExtensions) {
      if (strcmp(e.extensionName, "VK_KHR_surface_protected_capabilities") != 0)
        extensions.push_back(e.extensionName);
    }
    if (useDebug)extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    if (!cppvk::existSupport(validationLayers, instanceLayer)) {
      throw std::runtime_error("Error Validation Layers " );
    }


    m_instance = cppvk::InstanceBuilder()
      .applicationName("Hello Vulkan")
      .engineName("Vulkan Engine")
      .apiVersion(VK_API_VERSION_1_1)
      .engineVersion(VK_MAKE_VERSION(1, 0, 0))
      .enabledExtensionNames(extensions)
      .enabledLayerNames(validationLayers)
      .create();

#if _DEBUG
    m_debguUtilsMessenger = cppvk::DebugUtilsMessengerBuilder(m_instance)
      .severity(MessageServerity(VERBOSE) | MessageServerity(WARNING) | MessageServerity(ERROR))
      .type(MessageType(GENERAL) | MessageType(VALIDATION) | MessageType(PERFORMANCE))
      .callback(debugCallback)
      .create();
#endif

    m_surface = cppvk::SurfaceBuilder(m_instance)
      .hwnd(hwnd)
      .create();

    std::vector<VkPhysicalDevice> physicalDeviceList;
    cppvk::getEnumeratePhysicalDevices(m_instance.get(), physicalDeviceList);

    std::unique_ptr<cppvk::PhysicalDeviceDetails<std::vector>>  pPhysicalDetails;
    std::unique_ptr<cppvk::PhysicalDeviceSurfaceDetails<std::vector>> pPhysicalSurfaceDetails;
    auto pPhysicalDevice = std::find_if(std::begin(physicalDeviceList), std::end(physicalDeviceList), [&](VkPhysicalDevice& physicalDevice) {
      pPhysicalDetails = std::make_unique< cppvk::PhysicalDeviceDetails<std::vector> >(physicalDevice);
      pPhysicalSurfaceDetails = std::make_unique< cppvk::PhysicalDeviceSurfaceDetails<std::vector> >(physicalDevice, m_surface.get());
      return true;
     });

    if (pPhysicalDevice == std::end(physicalDeviceList)) {
      throw std::runtime_error("Physical device not found");
    }

    const auto graphics_queue_index = find_if_index(begin(pPhysicalDetails->queueProperties), end(pPhysicalDetails->queueProperties),
      [](VkQueueFamilyProperties queue) {return queue.queueFlags & VK_QUEUE_GRAPHICS_BIT; });

    if (graphics_queue_index == UINT32_MAX) {
      throw std::runtime_error("not  find VK_QUEUE_GRAPHICS_BIT.");
    }
    cppvk::Priorities default_queue_priority{ 1.0f };

    // set device extensions
    devExtension.clear();
    for (auto&& ext : pPhysicalDetails->extensions) {
      devExtension.push_back(ext.extensionName);
    }

   cppvk::DeviceQueueCreateInfoList<std::vector>  pDeviceQueueInfos {
     cppvk::DeviceQueueCreateInfoWrapper()
     .queuePriorities(default_queue_priority)
     .familyIndex(graphics_queue_index)
   };

    m_logicalDevice = cppvk::LogicalDeviceBuilder(*pPhysicalDevice)
      .extensions(devExtension)
      .layerNames(validationLayers)
      .features(pPhysicalDetails->features)
      .queueCreateInfos(pDeviceQueueInfos)
      .create();

    m_cmdPool = cppvk::CommandPoolBuilder(m_logicalDevice)
      .flags(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT)
      .queueFamilyIndices(graphics_queue_index)
      .create();

    m_cmdBuffer = cppvk::CommandBufferAllocate<CMD_BUFFER_SIZE>(m_logicalDevice)
      .commandPool(m_cmdPool)
      .level(VK_COMMAND_BUFFER_LEVEL_PRIMARY)
      .allocate();

    auto suitableFormat = std::find_if(std::begin(pPhysicalSurfaceDetails->formatList), std::end(pPhysicalSurfaceDetails->formatList), [](VkSurfaceFormatKHR format) {
      return format.format == VkFormat::VK_FORMAT_B8G8R8A8_UNORM;
      });
    if (suitableFormat == std::end(pPhysicalSurfaceDetails->formatList)) {
      throw std::runtime_error("No suitable Surface format found." );
    }

    auto suitablePresent = std::find_if(std::begin(pPhysicalSurfaceDetails->presentModeList), std::end(pPhysicalSurfaceDetails->presentModeList), [](VkPresentModeKHR mode) {
      return mode == VkPresentModeKHR::VK_PRESENT_MODE_FIFO_KHR;
      });
    if (suitablePresent == std::end(pPhysicalSurfaceDetails->presentModeList)) {
      throw std::runtime_error( "No suitable present mode found.");
    }

    cppvk::Indexs queue_family_indices = { graphics_queue_index };
    for (auto&& indice : queue_family_indices) {
      if (!cppvk::isSurfaeSupport(*pPhysicalDevice, m_surface.get(), indice))
        throw std::runtime_error("Unsupported index information.");
    }

  }
};

class VkApi :public  App {
  MyContext context;
public:
	virtual void Initialize() override {
		context.WinInit(winptr->getWin32(), 1280, 720);
	}

	virtual void Update() override {

	}

	virtual void Draw() const override {

	}

	virtual void Finalize() override {
	}

};

int main() {
	VkApi api;
	AppWindow ApplicationWindow(1280, 720, "Hello Vulkan Api", &api);
	ApplicationWindow.Run();
}