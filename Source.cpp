#include "Window/App.h"
#include "Window/AppWindow.h"

#include "cppvk/vk.h"
#include "cppvk/physicaldevice/physicaldevice.h"

#include "cppvk/info/devicequeueinfo.h"

#include "cppvk/objects/instance.h"
#include "cppvk/objects/debugutilsmessenger.h"
#include "cppvk/objects/surface.h"
#include "cppvk/objects/logicaldevice.h"

#include "cppvk/builders/instancebuilder.h"
#include "cppvk/builders/debugutilsmessengerbuilder.h"
#include "cppvk/builders/surfacebuilder.h"
#include "cppvk/builders/logicaldevicebuilder.h"

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

    const auto graphics_queue_index = find_if_index(physicalDevice->details.queueProperties.begin(), physicalDevice->details.queueProperties.end(), [](VkQueueFamilyProperties queue) {return queue.queueFlags & VK_QUEUE_GRAPHICS_BIT; });
    if (graphics_queue_index == UINT32_MAX)
      std::cerr << "not  find VK_QUEUE_GRAPHICS_BIT." << std::endl;
    cppvk::Priorities default_queue_priority{ 1.0f };
    // set device extensions
    devExtension.clear();
    for (auto&& ext : physicalDevice->details.extensions) {
      devExtension.push_back(ext.extensionName);
    }

    m_logicalDevice = cppvk::LogicalDeviceBuilder(m_instance, physicalDevice)
      .addQueueInfo(
        cppvk::DeviceQueueInfo()
        .queuePriorities(default_queue_priority)
        .familyIndex(graphics_queue_index))
      .extensions(devExtension)
      .layerNames(validationLayers)
      .features(physicalDevice->details.features)
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