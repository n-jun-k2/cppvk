#include "Window/App.h"
#include "Window/AppWindow.h"

#include "cppvk/vk.h"
#include "cppvk/physicaldevice/physicaldevice.h"

#include "cppvk/info/devicequeueinfo.h"

#include "cppvk/allocator/commandbuffer.h"

#include "cppvk/objects/instance.h"
#include "cppvk/objects/debugutilsmessenger.h"
#include "cppvk/objects/surface.h"
#include "cppvk/objects/logicaldevice.h"
#include "cppvk/objects/commandpool.h"
#include "cppvk/objects/swapchain.h"
#include "cppvk/objects/image.h"

#include "cppvk/builders/instancebuilder.h"
#include "cppvk/builders/debugutilsmessengerbuilder.h"
#include "cppvk/builders/surfacebuilder.h"
#include "cppvk/builders/logicaldevicebuilder.h"
#include "cppvk/builders/commandpoolbuilder.h"
#include "cppvk/builders/swapchainbuilder.h"
#include "cppvk/builders/imagebuilder.h"

#include <algorithm>
#include <set>

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

  cppvk::Instance::pointer m_instance;
  cppvk::DebugUtilsMessenger::pointer m_debguUtilsMessenger;
  cppvk::Surface::pointer m_surface;
  cppvk::LogicalDevice::pointer m_logicalDevice;
  cppvk::CommandPool::pointer m_commandPool;
  cppvk::Swapchain::pointer m_swapchain;

  cppvk::Image::pointer m_depthImage;

public:
  MyContext() = default;
  ~MyContext() = default;

  void  WinInit(HWND hwnd, const uint32_t& , const uint32_t& ) {

    auto useDebug = true;
    auto instanceExtensions = cppvk::getEnumerateInstanceExtension();
    auto instanceLayer = cppvk::getEnumerateInstanceLayer();

    cppvk::Names extensions{};
    cppvk::Names validationLayers{ "VK_LAYER_KHRONOS_validation" }; // VK_LAYER_LUNARG_standard_validation
    cppvk::Names devExtension = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    for (const auto& e : instanceExtensions) {
      if (strcmp(e.extensionName, "VK_KHR_surface_protected_capabilities") != 0)
        extensions.push_back(e.extensionName);
    }
    if (useDebug)extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    if (!cppvk::existSupport(validationLayers, instanceLayer))
      std::cerr << "Error Validation Layers " << std::endl;


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

    auto physicalDevice =  cppvk::PhysicalDevice::choosePhysicalDevice(m_instance, [=](cppvk::PhysicalDevice& dev) {
      const auto& prop = dev.details.properties;
      const auto surfaceDetails = dev.getSurfaceDetails(m_surface);

      return true;
      });

    const auto surfaceDetails = physicalDevice->getSurfaceDetails(m_surface);
    const auto graphics_queue_index = find_if_index(physicalDevice->details.queueProperties.begin(), physicalDevice->details.queueProperties.end(), [](VkQueueFamilyProperties queue) {return queue.queueFlags & VK_QUEUE_GRAPHICS_BIT; });
    if (graphics_queue_index == UINT32_MAX)
      std::cerr << "not  find VK_QUEUE_GRAPHICS_BIT." << std::endl;
    cppvk::Priorities default_queue_priority{ 1.0f };
    // set device extensions
    devExtension.clear();
    for (auto&& ext : physicalDevice->details.extensions) {
      devExtension.push_back(ext.extensionName);
    }

    m_logicalDevice = cppvk::LogicalDeviceBuilder(physicalDevice)
      .addQueueInfo(
        cppvk::DeviceQueueInfo()
        .queuePriorities(default_queue_priority)
        .familyIndex(graphics_queue_index))
      .extensions(devExtension)
      .layerNames(validationLayers)
      .features(physicalDevice->details.features)
      .create();

    m_commandPool = cppvk::CommandPoolBuilder(m_logicalDevice)
      .flags(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT)
      .queueFamilyIndices(graphics_queue_index)
      .create();

    m_commandPool->getCommandBufferAllocator()
      .level(VK_COMMAND_BUFFER_LEVEL_PRIMARY)
      .commandBufferCount(1)
      .allocate();

    auto suitableFormat = std::find_if(surfaceDetails.formatList.begin(), surfaceDetails.formatList.end(), [](VkSurfaceFormatKHR format) {
      return format.format == VkFormat::VK_FORMAT_B8G8R8A8_UNORM;
      });
    if (suitableFormat == surfaceDetails.formatList.end())
      std::cerr << "No suitable Surface format found." << std::endl;

    auto suitablePresent = std::find_if(surfaceDetails.presentModeList.begin(), surfaceDetails.presentModeList.end(), [](VkPresentModeKHR mode) {
      return mode == VkPresentModeKHR::VK_PRESENT_MODE_FIFO_KHR;
      });
    if (suitablePresent == surfaceDetails.presentModeList.end())
      std::cerr << "No suitable present mode found." << std::endl;

    cppvk::Indexs queue_family_indices = { graphics_queue_index };
    for (auto&& indice : queue_family_indices) {
      if (!physicalDevice->isSurfaeSupport(m_surface, indice))
        std::cerr << "Unsupported index information." << std::endl;
    }

    m_swapchain = cppvk::SwapchainBuilder(m_surface, m_logicalDevice)
      .minImageCount(surfaceDetails.capabilities.minImageCount)
      .imageFormat(suitableFormat->format)
      .imageColorSpace(suitableFormat->colorSpace)
      .imageExtent(surfaceDetails.capabilities.currentExtent)
      .imageSharingMode(VK_SHARING_MODE_EXCLUSIVE)
      .imageUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
      .imageArrayLayers(1)
      .queueFamilyIndices(queue_family_indices)
      .presentMode(*suitablePresent)
      .preTransform(VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
      .compositeAlpha(VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR)
      .clippedOn()
      .create();

    VkExtent3D depthExtent = {};
    depthExtent.depth = 1;
    depthExtent.width = surfaceDetails.capabilities.currentExtent.width;
    depthExtent.height = surfaceDetails.capabilities.currentExtent.height;

    m_depthImage = cppvk::ImageBuilder(m_logicalDevice)
      .tiling(VK_IMAGE_TILING_OPTIMAL)
      .sharingMode(VK_SHARING_MODE_EXCLUSIVE)
      .imageType(VK_IMAGE_TYPE_2D)
      .format(VK_FORMAT_D32_SFLOAT)
      .extent(depthExtent)
      .mipLevels(1)
      .usage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
      .samples(VK_SAMPLE_COUNT_1_BIT)
      .arrayLayers(1)
      .create();

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