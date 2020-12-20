#pragma once

#pragma warning(push)
#pragma warning(disable : 26812)
#pragma warning(disable : 4505)

#if defined(ANDROID) || defined (__ANDROID__)
  #define VK_USE_PLATFORM_ANDROID_KHR
#elif defined(_WIN32)
  #define VK_USE_PLATFORM_WIN32_KHR
#else
  #define VK_USE_PLATFORM_XCB_KHR
#endif

#include <vulkan/vulkan.h>

#define STR(x) #x
#undef min
#undef max

#define MessageServerity(str)		VK_DEBUG_UTILS_MESSAGE_SEVERITY_##str##_BIT_EXT
#define MessageType(str) VK_DEBUG_UTILS_MESSAGE_TYPE_##str##_BIT_EXT

#include <tuple>
#include <vector>
#include <fstream>
#include <functional>
#include <type_traits>


namespace cppvk {

  using Code = std::vector<char>;
  using Names = std::vector<const char*>;
  using Priorities = std::vector<float>;
  using PresentModes = std::vector<VkPresentModeKHR>;
  using SurfaceFormats = std::vector<VkSurfaceFormatKHR>;
  using QueueCreateInfos = std::vector<VkDeviceQueueCreateInfo>;
  using PhysicalDeviceList = std::vector<VkPhysicalDevice>;
  using LayerPropertiesList = std::vector<VkLayerProperties>;
  using ExtensionPropertiesList = std::vector<VkExtensionProperties>;
  using PhysicalDeviceGroupList = std::vector<VkPhysicalDeviceGroupProperties>;
  using PhysicalDeviceQueueProps = std::vector<VkQueueFamilyProperties>;
  using IsSuitableQueuePropFuncInstanceBuilder = std::function<bool(VkQueueFamilyProperties)>;

  void checkVk(const VkResult& result, const std::string& message = "") {
    if (result == VK_SUCCESS)return;
    std::cerr << "VkResult : " << result << std::endl;
    throw std::runtime_error(message);
  }

  static ExtensionPropertiesList getEnumerateInstanceExtension() {
    uint32_t size = 0;
    checkVk(vkEnumerateInstanceExtensionProperties(nullptr, &size, nullptr));

    ExtensionPropertiesList list(size);
    checkVk(vkEnumerateInstanceExtensionProperties(nullptr, &size, list.data()));
    return list;
  }

  static LayerPropertiesList getEnumerateInstanceLayer() {
    uint32_t size = 0;
    checkVk(vkEnumerateInstanceLayerProperties(&size, nullptr));
    LayerPropertiesList list(size);
    checkVk(vkEnumerateInstanceLayerProperties(&size, list.data()));
    return list;
  }

  static uint32_t getEnumerateInstanceVersion() {
    uint32_t version;
    checkVk(vkEnumerateInstanceVersion(&version));
    return version;
  }



  template<class _T, template<class... Args>class Container>
  static bool _existSupport(const Names& target, const Container<_T>& source, std::function<const char* (const _T&)> toString) noexcept {
    
    if (target.size() == 0) return true;

    bool isfound = false;
    for (const auto& t : target)
    {
      for (const auto& s : source)
      {
        isfound = strcmp(t, toString(s)) == 0;
        if (isfound)break;
      }
      if (!isfound)return false;
    }
    return true;
  }
  static bool existSupport(const Names& target, const ExtensionPropertiesList& source) {
    return _existSupport<VkExtensionProperties, std::vector>(target, source, [](VkExtensionProperties prop) {return prop.extensionName; });
  }
  static bool existSupport(const Names& target, const LayerPropertiesList& source) {
    return _existSupport<VkLayerProperties, std::vector>(target, source, [](VkLayerProperties prop) {return prop.layerName; });
  }

  static Code readFile(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::ate | std::ios::binary);

    if (!file.is_open())
      throw std::runtime_error("failed to open file!");

    auto fileSize = (size_t)file.tellg();
    Code buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
  }

}



#pragma warning(pop)