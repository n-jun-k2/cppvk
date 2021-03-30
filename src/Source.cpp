#pragma warning( disable : 4505 4189 26812)
#include "Window/App.h"
#include "Window/AppWindow.h"

#include "cppvk/vk.h"
#include "cppvk/allocationcallbacks.h"
#include "cppvk/pointer.h"
#include "cppvk/builders/instancebuilder.h"
#include "cppvk/builders/debugutilsmessengerbuilder.h"
#include "cppvk/builders/surfacebuilder.h"
#include "cppvk/builders/logicaldevicebuilder.h"
#include "cppvk/builders/commandpoolbuilder.h"
#include "cppvk/builders/swapchainbuilder.h"
#include "cppvk/builders/imagebuilder.h"
#include "cppvk/builders/imageviewbuilder.h"
#include "cppvk/info/devicequeueinfo.h"
#include "cppvk/allocator/commandbuffer.h"
#include "cppvk/allocator/devicememory.h"

#include "cppvk/glslangtools.h"

#include <algorithm>
#include <set>
#include <vector>



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
  cppvk::SurfacePtr m_surface;
  cppvk::DevicePtr m_logicalDevice;
  cppvk::CommandPoolPtr m_cmdPool;
  cppvk::CommandBufferPtr<CMD_BUFFER_SIZE> m_cmdBuffer;
  cppvk::SwapchainPtr m_swapchain;

  std::vector<VkImage> m_swapchain_images; // no destroy
  std::vector<cppvk::ImageViewPtr> m_swapchain_iamgeViews;

#if _DEPTH
  cppvk::DeviceMemoryPtr m_depthMemory;
  cppvk::ImagePtr m_depthImage;
  cppvk::ImageViewPtr m_depthImageView;
#endif

  cppvk::AllocationCallbacksPtr m_instance_callbacks;
  cppvk::AllocationCallbacksPtr m_surface_callbacks;
  cppvk::AllocationCallbacksPtr m_device_callbacks;
  cppvk::AllocationCallbacksPtr m_commandpool_callbacks;
  cppvk::AllocationCallbacksPtr m_swapchain_callbacks;


#if _DEBUG
  cppvk::DebugUtilsMessengerPtr m_debugUtilsMessenger;
  cppvk::AllocationCallbacksPtr m_debugUtilsMessanger_callbacks;
#endif

public:
  MyContext() = default;
  ~MyContext() = default;

  void  WinInit(HWND hwnd, const uint32_t& , const uint32_t& ) {

    m_instance_callbacks = cppvk::createPAllocator<const char*>("instance");
    m_surface_callbacks = cppvk::createPAllocator<const char*>("surface");
    m_device_callbacks = cppvk::createPAllocator<const char*>("device");
    m_commandpool_callbacks = cppvk::createPAllocator<const char*>("commandpool");
    m_swapchain_callbacks = cppvk::createPAllocator<const char*>("swapchain");

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
      .create(m_instance_callbacks);

#if _DEBUG
    m_debugUtilsMessanger_callbacks = cppvk::createPAllocator<const char*>("debugUtils");
    m_debugUtilsMessenger = cppvk::DebugUtilsMessengerBuilder(m_instance)
      .severity(MessageServerity(VERBOSE) | MessageServerity(WARNING) | MessageServerity(ERROR))
      .type(MessageType(GENERAL) | MessageType(VALIDATION) | MessageType(PERFORMANCE))
      .callback(debugCallback)
      .create(m_debugUtilsMessanger_callbacks);
#endif

    m_surface = cppvk::SurfaceBuilder(m_instance)
      .hwnd(hwnd)
      .create(m_surface_callbacks);

    std::vector<VkPhysicalDevice> physicalDeviceList;
    cppvk::getEnumeratePhysicalDevices(m_instance.get(), physicalDeviceList);

    std::unique_ptr<cppvk::PhysicalDeviceDetails<std::vector>> pPhysicalDetails;
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
    std::vector<float> default_queue_priority{ 1.0f };

    // set device extensions
    devExtension.clear();
    for (auto&& ext : pPhysicalDetails->extensions) {
      devExtension.push_back(ext.extensionName);
    }

    cppvk::DeviceQueueCreateInfoList<std::vector> pDeviceQueueInfos {
      cppvk::DeviceQueueCreateInfoWrapper()
      .queuePriorities(default_queue_priority)
      .familyIndex(graphics_queue_index)
    };

    m_logicalDevice = cppvk::LogicalDeviceBuilder(*pPhysicalDevice)
      .extensions(devExtension)
      .layerNames(validationLayers)
      .features(pPhysicalDetails->features)
      .queueCreateInfos(pDeviceQueueInfos)
      .create(m_device_callbacks);

    m_cmdPool = cppvk::CommandPoolBuilder(m_logicalDevice)
      .flags(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT)
      .queueFamilyIndices(graphics_queue_index)
      .create(m_commandpool_callbacks);

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

    m_swapchain = cppvk::SwapchainBuilder(m_logicalDevice)
      .surface(m_surface)
      .minImageCount(pPhysicalSurfaceDetails->capabilities.minImageCount)
      .imageFormat(suitableFormat->format)
      .imageColorSpace(suitableFormat->colorSpace)
      .imageExtent(pPhysicalSurfaceDetails->capabilities.currentExtent)
      .imageSharingMode(VK_SHARING_MODE_EXCLUSIVE)
      .imageUsage(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
      .imageArrayLayers(1)
      .queueFamilyIndices(queue_family_indices)
      .presentMode(*suitablePresent)
      .preTransform(VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
      .compositeAlpha(VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR)
      .clippedOn()
      .create(m_swapchain_callbacks);

    cppvk::getSwapchainImagesKHR(m_logicalDevice.get(), m_swapchain.get(), m_swapchain_images);

    m_swapchain_iamgeViews.reserve(m_swapchain_images.size());
    for(auto swapchainImage : m_swapchain_images) {
      m_swapchain_iamgeViews.push_back(
        cppvk::ImageViewBuilder(m_logicalDevice)
        .viewType(VK_IMAGE_VIEW_TYPE_2D)
        .image(swapchainImage)
        .format(suitableFormat->format)
        .components({
          VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G,
          VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A,
        })
        .subresourceRange({VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1})
        .create()
      );
    }

#if _DEPTH

    VkExtent3D depthExtent = {};
    cppvk::extent2Dto3D(pPhysicalSurfaceDetails->capabilities.currentExtent, depthExtent);

    std::vector<VkFormat> depthFormatList = {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT};
    const auto depthFormat = std::find_if(std::begin(depthFormatList), std::end(depthFormatList), [&](decltype(depthFormatList)::const_reference format){
      VkFormatProperties props;
      vkGetPhysicalDeviceFormatProperties(*pPhysicalDevice, format, &props);

      const VkFormatFeatureFlags target = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
      const VkFormatFeatureFlags feature = props.optimalTilingFeatures & target;
      return feature == target;
    });

    if (depthFormat == std::end(depthFormatList)) {
      throw std::runtime_error("No suitable DepthImage format found.");
    }

    m_depthImage = cppvk::ImageBuilder(m_logicalDevice)
      .tiling(VK_IMAGE_TILING_OPTIMAL)
      .sharingMode(VK_SHARING_MODE_EXCLUSIVE)
      .imageType(VK_IMAGE_TYPE_2D)
      .format(*depthFormat)
      .extent(depthExtent)
      .mipLevels(1)
      .usage(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
      .samples(VK_SAMPLE_COUNT_1_BIT)
      .arrayLayers(1)
      .create();

    VkMemoryRequirements depthImageRequirements = {};
    vkGetImageMemoryRequirements(m_logicalDevice.get(), m_depthImage.get(), &depthImageRequirements);

    const auto depthImageTypeIndex = pPhysicalDetails->findMemoryType(depthImageRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    m_depthMemory = cppvk::DeviceMemoryAllocate(m_logicalDevice)
      .typeIndex(depthImageTypeIndex)
      .size(depthImageRequirements.size)
      .allocate();

    cppvk::checkVk(vkBindImageMemory(m_logicalDevice.get(), m_depthImage.get(), m_depthMemory.get(), 0));

    m_depthImageView = cppvk::ImageViewBuilder(m_logicalDevice)
      .viewType(VK_IMAGE_VIEW_TYPE_2D)
      .image(m_depthImage)
      .format(VK_FORMAT_D32_SFLOAT)
      .components({
        VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G,
        VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A,
      })
      .subresourceRange({
        VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 0, 1
      })
      .create();

#endif

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
  return 1;
}