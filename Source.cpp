#include "Window/App.h"
#include "Window/AppWindow.h"

#include "cppvk/vk.h"
#include "cppvk/context.h"
#include "cppvk/physicaldeviceset.h"

#include "cppvk/builders/builder.h"
#include "cppvk/builders/instance.h"
#include "cppvk/builders/debugutilsmessanger.h"
#include "cppvk/builders/winsurface.h"

#include "cppvk/objects/object.h"
#include "cppvk/objects/instance.h"
#include "cppvk/objects/debugutilsmessanger.h"
#include "cppvk/objects/winsurface.h"


#pragma warning( disable : 4505 4189)

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


class MyContext {

  cppvk::Context::Ptr m_ctx;
  cppvk::Instance::Ptr m_instance;
  cppvk::DebugUtilsMessenger::Ptr m_debguUtilsMessenger;
  cppvk::WinSurface::Ptr m_surface;

public:
  MyContext() = default;
  ~MyContext() = default;

  void  WinInit(HWND wPtr, const uint32_t& , const uint32_t& ) {

    auto useDebug = true;
    auto ext = cppvk::getEnumerateInstanceExtension();
    auto lay = cppvk::getEnumerateInstanceLayer();

    cppvk::Names extensions{};
    cppvk::Names validation_layers{ "VK_LAYER_LUNARG_standard_validation" };
    cppvk::Names dev_extension = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    for (const auto& e : ext) {
      if (strcmp(e.extensionName, "VK_KHR_surface_protected_capabilities") != 0)
        extensions.push_back(e.extensionName);
    }
    if (useDebug)extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    if (!cppvk::existSupport(validation_layers, lay))
      std::cerr << "Error Validation Layers " << std::endl;

    m_ctx = cppvk::Context::make();

    m_instance = cppvk::InstanceBuilder(m_ctx)
      .applicationName("Hello Vulkan")
      .engineName("Vulkan Engine")
      .apiVersion(VK_API_VERSION_1_1)
      .engineVersion(VK_MAKE_VERSION(1, 0, 0))
      .enabledExtensionNames(extensions)
      .enabledLayerNames(validation_layers)
      .create();

#if _DEBUG
    m_debguUtilsMessenger = cppvk::DebugUtilsMessengerBuilder(m_ctx)
      .severity(MessageServerity(VERBOSE) | MessageServerity(WARNING) | MessageServerity(ERROR))
      .type(MessageType(GENERAL) | MessageType(VALIDATION) | MessageType(PERFORMANCE))
      .callback(debugCallback)
      .create();
#endif

    m_surface = cppvk::WinSurfaceBuilder(m_ctx)
      .hwnd(wPtr)
      .create();

    //auto physical_device = m_instance->chooseGpu([](cppvk::PhysicalDeviceSet& device_set) {
    //  return true;
    //  });

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