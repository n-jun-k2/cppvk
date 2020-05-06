#pragma once

#pragma warning(push)
#pragma warning(disable : 26812)
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#pragma warning(pop)

#include <vector>
#include <unordered_map>
#include <functional>
#include <fstream>
#include <optional>
#include <memory>
#include <cassert>
#include <string>

/*デバッグ用*/
#define STR(x) #x
//min maxマクロキャンセル
#undef min
#undef max

namespace cppvk {

	/**
	 * @brief 
	 * 
	 * @param result 
	 */
	void Check(const VkResult& result,const std::string& message = "")
	{
		if (result == VK_SUCCESS)return;
		std::cerr << "VkResult : " << result << std::endl;
		throw std::runtime_error(message);
	}

	
	template<class T, template<class cT, class A = std::allocator<cT>>class Container>
	void add(Container<T>& container, const std::function<void(T&)>& createFunc) {
		auto item = T{};
		createFunc(item);
		container.push_back(item);
	}

	
	using ExtensionPropertiesList = std::vector< VkExtensionProperties>;
	using LayerPropertiesList = std::vector< VkLayerProperties>;
	using PhysicalDeviceList = std::vector< VkPhysicalDevice>;
	using PhysicalDeviceGroupList = std::vector< VkPhysicalDeviceGroupProperties>;
	using ImageList = std::vector< VkImage>;
	using Names = std::vector<const char*>;
	using Indexs = std::vector<uint32_t>;
	using Priorities = std::vector<float>;
	using Code = std::vector<char>;
	using QueueCreateInfos = std::vector<VkDeviceQueueCreateInfo>;
	using SurfaceFormats = std::vector<VkSurfaceFormatKHR>;
	using PresentModes = std::vector<VkPresentModeKHR>;
	using PhysicalDeviceQueueProps = std::vector<VkQueueFamilyProperties>;
	using ChoosePhysicalDeviceFuncInstanceBuilder = std::function<bool(const VkPhysicalDeviceProperties, const VkPhysicalDeviceFeatures)>;
	using IsSuitableQueuePropFuncInstanceBuilder = std::function<bool(VkQueueFamilyProperties)>;
	
	template <typename T, typename... Args>class delete_wrap_ptr;

	template <typename T>using shared_pointer = std::shared_ptr<std::remove_pointer_t<T>>;
	using InstancePtr = shared_pointer <delete_wrap_ptr<VkInstance>>;
	using DebugMessengerPtr = shared_pointer < delete_wrap_ptr< VkDebugUtilsMessengerEXT, InstancePtr>>;
	using SurfacePtr = shared_pointer < delete_wrap_ptr< VkSurfaceKHR, InstancePtr>>;
	using PhysicalDevicePtr = shared_pointer< VkPhysicalDevice>;
	using DevicePtr = shared_pointer<delete_wrap_ptr<VkDevice>>;
	using SwapchainPtr = shared_pointer<delete_wrap_ptr<VkSwapchainKHR,DevicePtr>>;
	using RenderpassPtr = shared_pointer<delete_wrap_ptr<VkRenderPass,DevicePtr>>;
	using CommandPoolPtr = shared_pointer<delete_wrap_ptr<VkCommandPool,DevicePtr>>;
	using ImageViewPtr = shared_pointer<delete_wrap_ptr<VkImageView,DevicePtr>>;
	using PipelinePtr = shared_pointer<delete_wrap_ptr<VkPipeline,DevicePtr>>;
	using PipelineLayoutPtr = shared_pointer<delete_wrap_ptr<VkPipelineLayout,DevicePtr>>;
	using ShaderModulePtr = shared_pointer<delete_wrap_ptr<VkShaderModule, DevicePtr>>;
	using DescriptorPoolPtr = shared_pointer<delete_wrap_ptr<VkDescriptorPool, DevicePtr>>;
	using FramebufferPtr = shared_pointer<delete_wrap_ptr<VkFramebuffer, DevicePtr>>;

	/**
	 * @brief Object to own custom deleter
	 * 
	 * @tparam T 
	 * @tparam Args 
	 */
	template <typename T,typename... Args>
	class delete_wrap_ptr {
		using type = std::remove_pointer_t<T>;
		using argsType = std::tuple<Args...>;
		using deleteFunc = std::function<void(type*, Args...)>;

		deleteFunc cleanup;
		std::tuple<Args...> context;
		type* value;
	public:
		delete_wrap_ptr(type* item, deleteFunc func, Args... args)
			:value(item), cleanup(func), context({ args... }) {}
		~delete_wrap_ptr() {
			auto f = [&](Args ...args) {
				cleanup(value,args...);
			};
			std::apply(f,context);
		}
		type* get()const noexcept {
			return value;
		}
		type* operator*()const noexcept {
			return get();
		}
	};
	
	/**
	 * @brief Settings used in the swap chain
	 * 
	 */
	namespace SwapchainSupportDetails {
		using Type = std::tuple<VkSurfaceCapabilitiesKHR, SurfaceFormats, PresentModes>;

		static VkSurfaceCapabilitiesKHR getSurfaceCapabilities(const Type& data) {
			return std::get<0>(data);
		}
		static SurfaceFormats getSurfaceFormats(const Type& data) {
			return std::get<1>(data);
		}
		static PresentModes getPresentModes(const Type& data) {
			return std::get<2>(data);
		}
		static bool IsComplate(const Type& support) {
			return !std::get<1>(support).empty() && !std::get<2>(support).empty();
		}
	}

	/**
	 * @brief Object that stores the value of the queue family index for graphics and present mode
	 * 
	 */
	namespace QueueFamilyIndices {
		using Type = std::tuple<std::optional<uint32_t>, std::optional<uint32_t>>;

		static uint32_t GraphicsFamily(Type& indices) {
			return std::get<0>(indices).value();
		}
		static uint32_t PresentFamily(Type& indices) {
			return std::get<1>(indices).value();
		}
		static bool IsComplate(const Type& indices) {
			return std::get<0>(indices).has_value() && std::get<1>(indices).has_value();
		}
		static bool Match(const Type& indices) {
			return std::get<0>(indices).value() == std::get<1>(indices).value();
		}
	}

	namespace Alg {
		/* target文字列配列が全てsource配列の要素にあるのかの存在を確認する。存在した場合：真*/
		template<class _T, template<class... Args>class Container>
		static bool _ExistSupport(const Names& target, const Container<_T>& source, std::function<const char* (const _T&)> toString) noexcept
		{
			//そもそも比較必要がない場合。
			if (target.size() == 0) return true;

			bool isfound = false;
			for (const auto& t : target)/*確認したい項目*/
			{
				for (const auto& s : source)/*現環境の機能リストを走査*/
				{
					isfound = strcmp(t, toString(s)) == 0;
					if (isfound)break;
				}
				//一致していなかった場合
				if (!isfound)return false;
			}
			//全て走査できた場合
			return true;
		}

	}	

	/**
	 * @brief Vulkan API helper methos
	 * 
	 */
	struct helper {
		//Enumerator系
		/*物理デバイスの拡張機能一覧*/
		static ExtensionPropertiesList GetEnumeratePhysicalDeviceExtensions(const VkPhysicalDevice device) {

			assert(device != VK_NULL_HANDLE);

			uint32_t size = 0;
			Check(vkEnumerateDeviceExtensionProperties(device, nullptr, &size, nullptr));

			ExtensionPropertiesList list(size);
			Check(vkEnumerateDeviceExtensionProperties(device, nullptr, &size, list.data()));

			return list;
		}
		/*物理デバイスの検証機能一覧*/
		static LayerPropertiesList GetEnumerateDeviceLayer(const VkPhysicalDevice device) {
			assert(device != VK_NULL_HANDLE);
			uint32_t size = 0;
			Check(vkEnumerateDeviceLayerProperties(device, &size, nullptr));

			LayerPropertiesList list(size);
			Check(vkEnumerateDeviceLayerProperties(device, &size, list.data()));

			return list;
		}
		/*VulkanAPIの対応拡張機能一覧*/
		static ExtensionPropertiesList GetEnumerateInstanceExtension() {
			uint32_t size = 0;
			Check(vkEnumerateInstanceExtensionProperties(nullptr, &size, nullptr));

			ExtensionPropertiesList list(size);
			Check(vkEnumerateInstanceExtensionProperties(nullptr, &size, list.data()));
			return list;
		}
		/*VulkanAPIの対応検証機能一覧*/
		static LayerPropertiesList GetEnumerateInstanceLayer() {
			uint32_t size = 0;
			Check(vkEnumerateInstanceLayerProperties(&size, nullptr));
			LayerPropertiesList list(size);
			Check(vkEnumerateInstanceLayerProperties(&size, list.data()));
			return list;
		}
		/*VulkanAPIのバージョン*/
		static uint32_t GetEnumerateInstanceVersion() {
			uint32_t version;
			Check(vkEnumerateInstanceVersion(&version));
			return version;
		}
		/*物理デバイスの一覧*/
		static PhysicalDeviceList GetEnumeratePhysicalDevices(const VkInstance instance)
		{
			assert(instance != VK_NULL_HANDLE);
			uint32_t size;
			Check(vkEnumeratePhysicalDevices(instance, &size, nullptr));

			PhysicalDeviceList list(size);
			Check(vkEnumeratePhysicalDevices(instance, &size, list.data()));

			return list;
		}
		/*物理デバイスのグループ機能一覧*/
		static PhysicalDeviceGroupList GetEnumeratePhysicalDeviceGroupsKHR(const VkInstance instance) {
			assert(instance != VK_NULL_HANDLE);
			uint32_t size = 0;
			Check(vkEnumeratePhysicalDeviceGroupsKHR(instance, &size, nullptr));
			PhysicalDeviceGroupList list(size);
			Check(vkEnumeratePhysicalDeviceGroupsKHR(instance, &size, list.data()));
			return list;
		}
		/*物理デバイスのグループ機能一覧*/
		static PhysicalDeviceGroupList GetEnumeratePhysicalDeviceGroups(const VkInstance instance) {
			assert(instance != VK_NULL_HANDLE);
			uint32_t size = 0;
			Check(vkEnumeratePhysicalDeviceGroups(instance, &size, nullptr));
			PhysicalDeviceGroupList list(size);
			Check(vkEnumeratePhysicalDeviceGroups(instance, &size, list.data()));
			return list;
		}
		/*物理デバイスのキューファミリ一覧*/
		static PhysicalDeviceQueueProps GetEnumeratePhysicalDeviceQueueProps(const VkPhysicalDevice gpu) noexcept
		{
			assert(gpu != VK_NULL_HANDLE);
			uint32_t  count = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(gpu, &count, nullptr);

			PhysicalDeviceQueueProps queue(count);
			vkGetPhysicalDeviceQueueFamilyProperties(gpu, &count, queue.data());

			return queue;
		}
		/*サーフェイスの機能取得*/
		static VkSurfaceCapabilitiesKHR GetSurfaceCapabilities(VkPhysicalDevice gpu, VkSurfaceKHR surface)
		{
			VkSurfaceCapabilitiesKHR temp{};
			Check(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &temp));
			return temp;
		}
		/*物理デバイスのサーフェイスが利用可能なフォーマット*/
		static SurfaceFormats GetEnumerateSurfaceFormats(VkPhysicalDevice gpu, VkSurfaceKHR surface)
		{
			uint32_t fcount;
			vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &fcount, nullptr);
			SurfaceFormats formats(fcount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &fcount, formats.data());
			return formats;
		}
		/*物理デバイスのサーフェイスが利用可能なプレゼントモード*/
		static PresentModes GetEnumerateSurfacePresentmodes(VkPhysicalDevice gpu, VkSurfaceKHR surface)
		{
			uint32_t pcount;
			vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &pcount, nullptr);
			PresentModes presentModes(pcount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &pcount, presentModes.data());
			return presentModes;
		}
		//追加サポート機能
		static SwapchainSupportDetails::Type GetSwapchainSupport(VkPhysicalDevice gpu, VkSurfaceKHR surface)
		{
			VkSurfaceCapabilitiesKHR capabilites{};
			//基本的なサーフェイス情報取得
			vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &capabilites);
			//サーフェイスフォーマット情報取得
			auto formats = GetEnumerateSurfaceFormats(gpu, surface);
			//プレゼンテーション情報取得
			auto presentModes = GetEnumerateSurfacePresentmodes(gpu, surface);
			return { capabilites,formats,presentModes };
		}
		/*キューファミリのインデックスを返す。*/
		static QueueFamilyIndices::Type FindQueueFamilyIndices(VkPhysicalDevice gpu,VkSurfaceKHR surface)
		{
			assert(gpu != VK_NULL_HANDLE);
			assert(surface != VK_NULL_HANDLE);

			VkBool32 presentSupport = false;
			auto queues = GetEnumeratePhysicalDeviceQueueProps(gpu);

			int i = 0;
			std::optional<uint32_t> graphicsFamily , presentFamily;
			//プレゼンテーション探索対応。
			for (const auto& f : queues)
			{
				presentSupport = false;
				if(f.queueCount <= 0) continue;
				if (f.queueFlags & VK_QUEUE_GRAPHICS_BIT)graphicsFamily = i;
				
				Check(vkGetPhysicalDeviceSurfaceSupportKHR(gpu, i, surface, &presentSupport));
				if (presentSupport)presentFamily = i;
				if (QueueFamilyIndices::IsComplate({ graphicsFamily,presentSupport }))break;
				i++;
			}
			return {graphicsFamily,presentSupport};
		}
		/*デバイスキューのクリエイトオブジェクト*/
		static VkDeviceQueueCreateInfo QueueCreateInfo(const uint32_t familyIndex, const uint32_t count = 1, const float priority = 1.0f) {
			assert(count > 0);
			VkDeviceQueueCreateInfo info{};
			//info.sType = VK_CREATE_TYPE(DEVICE_QUEUE);
			info.flags = 0;
			info.pNext = VK_NULL_HANDLE;
			info.queueCount = count;
			info.queueFamilyIndex = familyIndex;
			info.pQueuePriorities = &priority;
			return info;
		}
		/* source配列にtarget名が全て存在するか。 */
		static bool ExistSupport(const Names& target, const ExtensionPropertiesList& source) {
			return Alg::_ExistSupport<VkExtensionProperties,std::vector>(target, source, [](VkExtensionProperties prop) {return prop.extensionName;});
		}
		/* source配列にtarget名が全て存在するか。 */
		static bool ExistSupport(const Names& target, const LayerPropertiesList& source) {
			return Alg::_ExistSupport<VkLayerProperties, std::vector>(target, source, [](VkLayerProperties prop) {return prop.layerName;});
		}
		/*削除処理をする必要がない為そのままデバイスポインタを返す。*/
		static VkPhysicalDevice chooseGpu(InstancePtr instance, ChoosePhysicalDeviceFuncInstanceBuilder isSuitable) {

			VkPhysicalDeviceProperties dprop; //名前,種類,サポート,バージョンなど
			VkPhysicalDeviceFeatures   dfeat;//テクスチャ圧縮,64ビットfloat,マルチビューポートレンダリングなどのオプション
			
			PhysicalDeviceList gpus = GetEnumeratePhysicalDevices(**instance);

			for (auto&& gpu : gpus) {
				vkGetPhysicalDeviceProperties(gpu, &dprop);
				vkGetPhysicalDeviceFeatures(gpu, &dfeat);

				/*最適条件ではない場合繰り返す。*/
				if (!isSuitable(dprop, dfeat))	continue;

				return gpu;
			}
			return VK_NULL_HANDLE;
		}
		/*削除処理をする必要がない為そのままキューポインタを返す。*/
		static VkQueue chooseQueue(DevicePtr device, uint32_t family, uint32_t index) {
			VkQueue queue;
			vkGetDeviceQueue(**device, family, index, &queue);
			return queue;
		}
		/*サーフェイスの優先すべきフォーマット情報を返す。*/
		static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
			//サーフィス情報に優先フォーマットがない場合
			if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED)
				return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

			//フォーマットに縛りがある場合はリストを探索し、希望の組み合わせが可能かを確認.
			for (const auto& availableFormat : availableFormats)
			{
				if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM &&
					availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
					return availableFormat;
			}

			return availableFormats[0];
		}
		/*サーフェイスの優先すべきフォーマット情報を返す。*/
		static VkSurfaceFormatKHR chooseSwapSurfaceFormat(SwapchainSupportDetails::Type& support) {
			return chooseSwapSurfaceFormat(SwapchainSupportDetails::getSurfaceFormats(support));
		}
		/*プレゼントモードの優先すべきモードを返す。*/
		static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
		{
			for (const auto& present : availablePresentModes)
			{
				if (present == VK_PRESENT_MODE_MAILBOX_KHR)return present;
			}
			return VK_PRESENT_MODE_FIFO_KHR;
		}
		/*プレゼントモードの優先すべきモードを返す。*/
		static VkPresentModeKHR chooseSwapPresentMode(SwapchainSupportDetails::Type& support) {
			return chooseSwapPresentMode(SwapchainSupportDetails::getPresentModes(support));
		}
		/*範囲データが異常値の場合空を返す。*/
		static VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilites) {
			if (capabilites.currentExtent.width != std::numeric_limits<uint32_t>::max())return capabilites.currentExtent;

			VkExtent2D extent2d = {};
			return extent2d;
		}
		/*範囲データが異常値の場合空を返す。*/
		static VkExtent2D chooseSwapExtent(SwapchainSupportDetails::Type& support) {
			return chooseSwapExtent(SwapchainSupportDetails::getSurfaceCapabilities(support));
		}
		/*スワップチェーンの画像取得 */
		static ImageList GetSwapchainImage(VkDevice device, VkSwapchainKHR swapchain) noexcept{
			uint32_t count;
			ImageList images;

			if(device == VK_NULL_HANDLE || swapchain == VK_NULL_HANDLE)
				return images;

			vkGetSwapchainImagesKHR(device, swapchain, &count, nullptr);

			images.resize(count);
			vkGetSwapchainImagesKHR(device, swapchain, &count, images.data());

			return images;
		}

		static Code readFile(const std::string& filePath) {
			std::ifstream file(filePath, std::ios::ate | std::ios::binary);

			if (!file.is_open()) 
				throw std::runtime_error("failed to open file!");
			
			auto fileSize = (size_t) file.tellg();
			Code buffer(fileSize);

			file.seekg(0);
			file.read(buffer.data(), fileSize);
			file.close();

			return buffer;
		}

	};
	
	/**
	 * @brief 
	 * 
	 */
	class InstanceBuilder
	{
			VkInstanceCreateInfo info;
			VkApplicationInfo appInfo;
	public:
			static InstanceBuilder get() {
				return {};
			}
			~InstanceBuilder() {}
			InstanceBuilder(){				
				appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
				appInfo.pNext = VK_NULL_HANDLE;
				appInfo.pApplicationName = NULL;
				appInfo.pEngineName = NULL;

				info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
				info.pNext = VK_NULL_HANDLE;
				info.flags = 0;
				info.pApplicationInfo = &appInfo;
				info.enabledLayerCount = 0;
				info.ppEnabledLayerNames = NULL;
				info.enabledExtensionCount = 0;
				info.ppEnabledExtensionNames = NULL;
			}
			/**
			 * @brief pApplicationName is NULL or is a pointer to a null-terminated UTF-8 string containing the name of the application.
			 * 
			 * @param name 
			 * @return InstanceBuilder 
			 */
			InstanceBuilder applicationName(const std::string& name){
				appInfo.pApplicationName = name.c_str();
				return *this;
			}

			/**
			 * @brief pEngineName is NULL or is a pointer to a null-terminated UTF-8 string containing the name of the engine (if any) used to create the application.
			 * 
			 * @param name 
			 * @return InstanceBuilder 
			 */
			InstanceBuilder engineName(const std::string& name){
				appInfo.pEngineName = name.c_str();
				return *this;
			}

			/**
			 * @brief applicationVersion is an unsigned integer variable containing the developer-supplied version number of the application.
			 * 
			 * @param version 
			 * @return InstanceBuilder 
			 */
			InstanceBuilder applicationVersion(const uint32_t version){
				appInfo.applicationVersion = version;
				return *this;
			}

			/**
			 * @brief engineVersion is an unsigned integer variable containing the developer-supplied version number of the engine used to create the application.
			 * 
			 * @param version 
			 * @return InstanceBuilder 
			 */
			InstanceBuilder engineVersion(const uint32_t version){
				appInfo.engineVersion = version;
				return *this;
			}

			/**
			 * @brief apiVersion is the version of the Vulkan API against which the application expects to run, encoded as described in the API Version Numbers and Semantics section. If apiVersion is 0 the implementation must ignore it, otherwise if the implementation does not support the requested apiVersion, or an effective substitute for apiVersion, it must return VK_ERROR_INCOMPATIBLE_DRIVER. The patch version number specified in apiVersion is ignored when creating an instance object. Only the major and minor versions of the instance must match those requested in apiVersion.
			 * 
			 * @param version 
			 * @return InstanceBuilder 
			 */
			InstanceBuilder apiVersion(const uint32_t version){
				appInfo.apiVersion = version;
				return *this;
			}

			/**
			 * @brief ppEnabledLayerNames is a pointer to an array of enabledLayerCount null-terminated UTF-8 strings containing the names of layers to enable for the created instance. See the Layers section for further details.
			 *
			 * @param layers 
			 * @return InstanceBuilder 
			 */
			InstanceBuilder enabledLayerNames(const Names& layers){
				info.enabledLayerCount = static_cast<uint32_t>(layers.size());
				if(!layers.empty())info.ppEnabledLayerNames = layers.data();
				return *this;
			}

			/**
			 * @brief ppEnabledExtensionNames is a pointer to an array of enabledExtensionCount null-terminated UTF-8 strings containing the names of extensions to enable.
			 * 
			 * @param extensions 
			 * @return InstanceBuilder 
			 */
			InstanceBuilder enabledExtensionNames(const Names& extensions){
				info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
				if(!extensions.empty())info.ppEnabledExtensionNames = extensions.data();
				return *this;
			}

			/**
			 * @brief 
			 * 
			 * @return InstanceInstanceBuilder 
			 */
			InstancePtr build(){
				VkInstance instance;
				auto err = vkCreateInstance(&info, VK_NULL_HANDLE, &instance);
				Check(err);
				return std::make_shared<delete_wrap_ptr<VkInstance>>(instance, [](VkInstance ptr) {
					std::cout << STR(vkDestroyInstance) << std::endl;
					vkDestroyInstance(ptr, VK_NULL_HANDLE);
					});
			}

	};//InstanceBuilder
	
	/**
	 * @brief 
	 * 
	 */
	class DebugUtilsMessengerBuilder {
		VkDebugUtilsMessengerCreateInfoEXT info;
		InstancePtr instance;

		static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pInfo,
			const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
		{
			auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
			if (func != nullptr)return func(instance, pInfo, pAllocator, pDebugMessenger);
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}

		static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
		{
			auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
			if (func != nullptr)func(instance, debugMessenger, pAllocator);
		}
	
	public:
		/**
		 * @brief Construct a new Debug Utils Messenger Builder object
		 * 
		 * @param instanceptr 
		 */
		DebugUtilsMessengerBuilder(InstancePtr instanceptr) :instance(instanceptr){
			info = {};
			info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			info.pNext = NULL;
			info.flags = 0;
			info.pUserData = nullptr;
		}
		~DebugUtilsMessengerBuilder() {}

		/**
		 * @brief 
		 * 
		 * @param instanceptr 
		 * @return DebugUtilsMessengerBuilder 
		 */
		static DebugUtilsMessengerBuilder get(InstancePtr instanceptr) {
			return DebugUtilsMessengerBuilder{instanceptr};
		}

		/**
		 * @brief 
		 * 
		 * @param value 
		 * @return DebugUtilsMessengerBuilder 
		 */
		DebugUtilsMessengerBuilder severity(VkDebugUtilsMessageSeverityFlagsEXT value) {
			info.messageSeverity = value;
			return *this;
		}

		/**
		 * @brief 
		 * 
		 * @param value 
		 * @return DebugUtilsMessengerBuilder 
		 */
		DebugUtilsMessengerBuilder type(VkDebugUtilsMessageTypeFlagsEXT value) {
			info.messageType = value;
			return *this;
		}

		/**
		 * @brief 
		 * 
		 * @param value 
		 * @return DebugUtilsMessengerBuilder 
		 */
		DebugUtilsMessengerBuilder callback(PFN_vkDebugUtilsMessengerCallbackEXT value) {
			info.pfnUserCallback = value;
			return *this;
		}

		/**
		 * @brief 
		 * 
		 * @return DebugMessengerPtr 
		 */
		DebugMessengerPtr build() {
			VkDebugUtilsMessengerEXT debugMsg = VK_NULL_HANDLE;

			auto err = CreateDebugUtilsMessengerEXT(**instance, &info, VK_NULL_HANDLE, &debugMsg);
			Check(err);
			return std::make_shared< delete_wrap_ptr<VkDebugUtilsMessengerEXT, InstancePtr>>(
				debugMsg, 
				[](VkDebugUtilsMessengerEXT ptr, InstancePtr ins ) {
					std::cout << STR(vkDestroyDebugUtilsMessengerEXT) << std::endl;
					DestroyDebugUtilsMessengerEXT(**ins, ptr, VK_NULL_HANDLE);
				}, 
				instance);
		}
	};//DebugMessengerPtr

	/**
	 * @brief 
	 * 
	 */
	class WinSurfaceBuilder {
		VkWin32SurfaceCreateInfoKHR info = {};
		InstancePtr instance;
	public:
		/**
		 * @brief Construct a new Win Surface Builder object
		 * 
		 * @param ptr 
		 */
		WinSurfaceBuilder(InstancePtr ptr) :instance(ptr){
			info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
			info.hinstance = GetModuleHandle(nullptr);
			info.pNext = NULL;
			info.flags = 0;
		}

		/**
		 * @brief 
		 * 
		 * @param ptr 
		 * @return WinSurfaceBuilder 
		 */
		static WinSurfaceBuilder get(InstancePtr ptr) {
			return {ptr};
		}

		/**
		 * @brief 
		 * 
		 * @param item 
		 * @return WinSurfaceBuilder 
		 */
		WinSurfaceBuilder hwnd(HWND item) {
			info.hwnd = item;
			return *this;
		}

		/**
		 * @brief 
		 * 
		 * @param item 
		 * @return WinSurfaceBuilder 
		 */
		WinSurfaceBuilder hinstance(HINSTANCE item) {
			info.hinstance = item;
			return *this;
		}

		/**
		 * @brief 
		 * 
		 * @return SurfacePtr 
		 */
		SurfacePtr build() {
			VkSurfaceKHR surface = VK_NULL_HANDLE;
			auto err = vkCreateWin32SurfaceKHR(**instance, &info, VK_NULL_HANDLE, &surface);
			Check(err);

			return std::make_shared<  delete_wrap_ptr < VkSurfaceKHR , InstancePtr> >( 
				surface,
				[](VkSurfaceKHR ptr, InstancePtr ins) {
					std::cout << STR(vkDestroySurfaceKHR) << std::endl;
					vkDestroySurfaceKHR(**ins, ptr, VK_NULL_HANDLE);
				},
				instance);
		}
	};//WinSurfaceBuilder

	/**
	 * @brief 
	 * 
	 */
	class DeviceBuilder{
		VkDeviceCreateInfo info;
		VkPhysicalDevice selectDevice;
		QueueCreateInfos queueInfos;
		public:
			/**
			 * @brief Construct a new Device Builder object
			 * 
			 * @param gpu 
			 */
			DeviceBuilder(VkPhysicalDevice gpu):selectDevice(gpu){
				info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
				info.pNext = NULL;
				info.flags = 0;
				info.queueCreateInfoCount = 0;
				info.enabledLayerCount = 0;
				info.enabledExtensionCount = 0;
			}
			~DeviceBuilder(){}

			/**
			 * @brief 
			 * 
			 * @param gpu 
			 * @return DeviceBuilder 
			 */
			static DeviceBuilder get(VkPhysicalDevice gpu) {
				return DeviceBuilder(gpu);
			}

			/**
			 * @brief 
			 * 
			 */
			class DeviceQueueSetter {
				VkDeviceQueueCreateInfo &info;
			public:
				DeviceQueueSetter(VkDeviceQueueCreateInfo& i):info(i) {
					info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
					info.pNext = NULL;
					info.flags = 0;
					info.queueFamilyIndex = 0;
					info.queueCount = 0;
				}
				~DeviceQueueSetter() {}
				/**
				 * @brief queueFamilyIndex is an unsigned integer indicating the index of the queue family to create on this device. This index corresponds to the index of an element of the pQueueFamilyProperties array that was returned by vkGetPhysicalDeviceQueueFamilyProperties
				 *
				 * @param index
				 * @return DeviceQueueSetter
				 */
				DeviceQueueSetter familyIndices(const uint32_t index) {
					info.queueFamilyIndex = index;
					return *this;
				}
				/**
				 * @brief pQueuePriorities is an array of queueCount normalized floating point values, specifying priorities of work that will be submitted to each created queue. See Queue Priority for more information.
				 * lvalue only
				 * @param prior
				 * @return DeviceQueueSetter
				 */
				DeviceQueueSetter priorities(Priorities& prior) {
					info.queueCount = static_cast<uint32_t>(prior.size());
					if (!prior.empty())info.pQueuePriorities = prior.data();
					return *this;
				}
			};

			/**
			 * @brief 
			 * 
			 * @param layers 
			 * @return DeviceBuilder 
			 */
			DeviceBuilder layerNames(const Names& layers){
				info.enabledLayerCount = static_cast<uint32_t>(layers.size());
				if(!layers.empty())info.ppEnabledLayerNames = layers.data();
				return *this;
			}

			/**
			 * @brief 
			 * 
			 * @param extensions 
			 * @return DeviceBuilder 
			 */
			DeviceBuilder extensions(const Names& extensions){
				info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
				if(!extensions.empty())info.ppEnabledExtensionNames = extensions.data();
				return *this;
			}

			/**
			 * @brief 
			 * 
			 * @param list 
			 * @return DeviceBuilder 
			 */
			DeviceBuilder features(const VkPhysicalDeviceFeatures& list){
				info.pEnabledFeatures = &list;
				return *this;
			}

			/**
			 * @brief 
			 * 
			 * @param createFunc 
			 * @return DeviceBuilder 
			 */
			DeviceBuilder addQueueInfo(const std::function<void(DeviceQueueSetter&)>& createFunc) {
				auto item = VkDeviceQueueCreateInfo{};
				auto op = DeviceQueueSetter(item);
				createFunc(op);
				queueInfos.push_back(item);
				return *this;
			}

			/**
			 * @brief 
			 * 
			 * @return DevicePtr 
			 */
			DevicePtr build() {
				/* キュー作成情報の更新 */
				info.queueCreateInfoCount = static_cast<uint32_t>(queueInfos.size());
				if(!queueInfos.empty())
					info.pQueueCreateInfos = queueInfos.data();

				VkDevice logicalDevice;
				/*論理デバイスの作成(同時にキューも作成される.キューは論理デバイスの削除で自動)*/
				auto err = vkCreateDevice(selectDevice, &info,VK_NULL_HANDLE,&logicalDevice);
				Check(err);

				return std::make_shared<delete_wrap_ptr<VkDevice>>( logicalDevice, [](VkDevice ptr) {
					std::cout << STR(vkDestroyDevice) << std::endl;
					vkDestroyDevice(ptr,VK_NULL_HANDLE);
				} );
			}
	};//DeviceBuilder

	/**
	 * @brief 
	 * 
	 */
	class SwapchainBuilder {
		VkSwapchainCreateInfoKHR info;
		DevicePtr logicalDevice;
	public:
		/**
		 * @brief Construct a new Swapchain Builder object
		 * 
		 * @param pointer 
		 */
		SwapchainBuilder(DevicePtr pointer):logicalDevice(pointer) {
			info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
			info.pNext = VK_NULL_HANDLE;
			info.flags = 0;
			info.queueFamilyIndexCount = 0;
			info.pQueueFamilyIndices = nullptr;
		}
		/**
		 * @brief 
		 * 
		 * @param pointer 
		 * @return SwapchainBuilder 
		 */
		static SwapchainBuilder get(DevicePtr pointer) {
			return SwapchainBuilder{pointer};
		}

		/**
		 * @brief 
		 * 
		 * @param pointer 
		 * @return SwapchainBuilder 
		 */
		SwapchainBuilder surface(SurfacePtr pointer) {
			info.surface = **pointer;
			return *this;
		}

		/**
		 * @brief 
		 * 
		 * @param count 
		 * @return SwapchainBuilder 
		 */
		SwapchainBuilder minImageCount(const uint32_t& count) {
			info.minImageCount = count;
			return *this;
		}

		/**
		 * @brief 
		 * 
		 * @param indices 
		 * @return SwapchainBuilder 
		 */
		SwapchainBuilder queueFamily(Indexs& indices) {
			info.queueFamilyIndexCount = static_cast<uint32_t>(indices.size());
			if (!indices.empty())
				info.pQueueFamilyIndices = indices.data();
			return *this;
		}

		/**
		 * @brief 
		 * 
		 * @param mode 
		 * @return SwapchainBuilder 
		 */
		SwapchainBuilder imageSharingMode(const VkSharingMode mode) {
			info.imageSharingMode = mode;
			return *this;
		}

		/**
		 * @brief 
		 * 
		 * @param format 
		 * @return SwapchainBuilder 
		 */
		SwapchainBuilder imageFormat(const VkFormat format) {
			info.imageFormat = format;
			return *this;
		}

		/**
		 * @brief 
		 * 
		 * @param extent 
		 * @return SwapchainBuilder 
		 */
		SwapchainBuilder imageExtent(const VkExtent2D extent) {
			info.imageExtent = extent;
			return *this;
		}

		/**
		 * @brief 
		 * 
		 * @param color 
		 * @return SwapchainBuilder 
		 */
		SwapchainBuilder imageColorSpace(const VkColorSpaceKHR color) {
			info.imageColorSpace = color;
			return *this;
		}

		/**
		 * @brief 
		 * 
		 * @param layers 
		 * @return SwapchainBuilder 
		 */
		SwapchainBuilder imageArrayLayers(const uint32_t& layers) {
			info.imageArrayLayers = layers;
			return *this;
		}

		/**
		 * @brief 
		 * 
		 * @param flag 
		 * @return SwapchainBuilder 
		 */
		SwapchainBuilder imageUsage(const VkImageUsageFlags flag) {
			info.imageUsage = flag;
			return *this;
		}

		/**
		 * @brief 
		 * 
		 * @param flag 
		 * @return SwapchainBuilder 
		 */
		SwapchainBuilder preTransform(const VkSurfaceTransformFlagBitsKHR& flag) {
			info.preTransform = flag;
			return *this;
		}

		/**
		 * @brief 
		 * 
		 * @param flag 
		 * @return SwapchainBuilder 
		 */
		SwapchainBuilder compositeAlpha(const VkCompositeAlphaFlagBitsKHR& flag) {
			info.compositeAlpha = flag;
			return *this;
		}

		/**
		 * @brief 
		 * 
		 * @param flag 
		 * @return SwapchainBuilder 
		 */
		SwapchainBuilder presentMode(const VkPresentModeKHR& flag) {
			info.presentMode = flag;
			return *this;
		}

		/**
		 * @brief 
		 * 
		 * @param swapchain 
		 * @return SwapchainBuilder 
		 */
		SwapchainBuilder oldSwapchain(VkSwapchainKHR& swapchain) {
			info.oldSwapchain = swapchain;
			return *this;
		}

		/**
		 * @brief 
		 * 
		 * @return SwapchainBuilder 
		 */
		SwapchainBuilder oldSwapchainNullSet() {
			info.oldSwapchain = VK_NULL_HANDLE;
			return *this;
		}

		/**
		 * @brief 
		 * 
		 * @return SwapchainBuilder 
		 */
		SwapchainBuilder clippedOn() {
			info.clipped = VK_TRUE;
			return *this;
		}

		/**
		 * @brief 
		 * 
		 * @return SwapchainBuilder 
		 */
		SwapchainBuilder clippedOff() {
			info.clipped = VK_FALSE;
			return *this;
		}

		/**
		 * @brief 
		 * 
		 * @return SwapchainPtr 
		 */
		SwapchainPtr build() {
			VkSwapchainKHR swapchain = VK_NULL_HANDLE;
			auto err = vkCreateSwapchainKHR(**logicalDevice, &info, VK_NULL_HANDLE, &swapchain);
			Check(err);

			return std::make_shared<delete_wrap_ptr<VkSwapchainKHR,DevicePtr>>(
				swapchain,
				[](VkSwapchainKHR ptr,DevicePtr device) {
					std::cout << STR(vkDestroySwapchainKHR) << std::endl;
					vkDestroySwapchainKHR(**device,ptr,VK_NULL_HANDLE);
				},
				logicalDevice);
		}
	};//SwapchainBuilder

	/**
	 * @brief 
	 * 
	 */
	class RenderpassBuilder{
		VkRenderPassCreateInfo info = {};
		DevicePtr logicalDevice;
		std::vector<VkAttachmentDescription> attachments;
		std::vector<VkSubpassDescription> description;
		std::vector<VkSubpassDependency> dependency;

	public:
		RenderpassBuilder(DevicePtr pointer):logicalDevice(pointer){
			info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			info.pNext = VK_NULL_HANDLE;
			info.flags = 0;
		}

		/**
		 * @brief 
		 * 
		 * @param pointer 
		 * @return RenderpassBuilder 
		 */
		static RenderpassBuilder get(DevicePtr pointer){
			return RenderpassBuilder(pointer);
		}

		/**
		 * @brief 
		 *  flags:VkAttachmentDescriptionの追加プロパティを指定する VkAttachmentDescriptionFlagBits のビットマスクです。
		 *  format:VkAttachmentDescriptionに使用される画像ビューのフォーマットを指定する VkFormat 値です。
		 *  samples:VkSampleCountFlagBits で定義されている画像のサンプル数です。
		 *  loadOp: VkAttachmentLoadOp 値で、最初に使用されるサブパスの開始時に、アタッチメントのカラー コンポーネントとデプス コンポーネントのコンテンツがどのように処理されるかを指定します。
		 *  storeOp: VkAttachmentStoreOp 値で、最後に使用したサブパスの終了時に、アタッチメントのカラー コンポーネントとデプス コンポーネントの内容がどのように処理されるかを指定します。
		 *  stencilLoadOp: VkAttachmentLoadOp 値で、アタッチメントのステンシル コンポーネントのコンテンツが、最初に使用されたサブパスの先頭でどのように処理されるかを指定します。
		 *  stencilStoreOp: VkAttachmentStoreOp 値で、アタッチメントのステンシル コンポーネントのコンテンツが、使用される最後のサブパスの最後にどのように処理されるかを指定します。
		 *  initialLayout: レンダーパスのインスタンスが開始されたときに、アタッチメントイメージのサブリソースが置かれるレイアウトです。
		 *  finalLayout: レンダーパスインスタンスが終了したときにアタッチメントイメージサブリソースが遷移するレイアウトです。
		 * @param createFunc 
		 * @return RenderpassBuilder 
		 */
		RenderpassBuilder addAttachments(const std::function<void(VkAttachmentDescription&)> createFunc){
			add<VkAttachmentDescription>(attachments,createFunc);
			return *this;
		}

		/**
		 * @brief 
		 * 
		 * @param 
		 * @return RenderpassBuilder 
		 */
		RenderpassBuilder addSubpassDescription(const std::function<void(VkSubpassDescription&)> createFunc){
			add<VkSubpassDescription>(description,createFunc);
			return *this;
		}

		/**
		 * @brief 
		 * 
		 * @param 
		 * @return RenderpassBuilder 
		 */
		RenderpassBuilder addSubpassDependency(const std::function<void(VkSubpassDependency&)> createFunc){
			add<VkSubpassDependency>(dependency,createFunc);
			return *this;
		}

		/**
		 * @brief 
		 * 
		 * @return RenderpassPtr 
		 */
		RenderpassPtr build(){

			info.attachmentCount = static_cast<uint32_t>(attachments.size());
			if(!attachments.empty())
				info.pAttachments = attachments.data();

			info.subpassCount = static_cast<uint32_t>(description.size());
			if (!description.empty())
				info.pSubpasses = description.data();

			info.dependencyCount = static_cast<uint32_t>(dependency.size());
			if (!dependency.empty())
				info.pDependencies = dependency.data();

			VkRenderPass renderPass = VK_NULL_HANDLE;
			auto err = vkCreateRenderPass(**logicalDevice,&info,VK_NULL_HANDLE,&renderPass);
			Check(err);

			return  std::make_shared<delete_wrap_ptr<VkRenderPass, DevicePtr>>(
				renderPass,
				[](VkRenderPass ptr, DevicePtr device) {
					std::cout << STR(vkDestroyRenderPass) << std::endl;
					vkDestroyRenderPass(**device, ptr, VK_NULL_HANDLE);
				},
				logicalDevice);

		}

	};//RenderpassBuilder

	class CommandPoolBuilder{
		VkCommandPoolCreateInfo info = {};
		DevicePtr logicalDevice;
	public:
		CommandPoolBuilder(DevicePtr pointer):logicalDevice(pointer){
			info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			info.pNext = VK_NULL_HANDLE;
			info.flags = 0;
		}

		static CommandPoolBuilder get(DevicePtr pointer){
			return CommandPoolBuilder(pointer);
		}

		CommandPoolBuilder queueFamilyIndices(const uint32_t& value){
			info.queueFamilyIndex = value;
			return *this;
		}

		CommandPoolPtr build(){
			VkCommandPool cmdPool = VK_NULL_HANDLE;
			auto err = vkCreateCommandPool(**logicalDevice,&info,VK_NULL_HANDLE,&cmdPool);
			Check(err);

			return std::make_shared<delete_wrap_ptr<VkCommandPool, DevicePtr>>(
				cmdPool,
				[](VkCommandPool ptr, DevicePtr device) {
					std::cout << STR(vkDestroyCommandPool) << std::endl;
					vkDestroyCommandPool(**device, ptr, VK_NULL_HANDLE);
				},
				logicalDevice);
		}
	};//RenderpassBuilder

	class ImageViewBuilder{
		VkImageViewCreateInfo info = {};
		DevicePtr logicalDevice;
	public:
		ImageViewBuilder(DevicePtr pointer):logicalDevice(pointer){
			info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			info.pNext = VK_NULL_HANDLE;
			info.flags = 0;
		}

		~ImageViewBuilder(){}

		static ImageViewBuilder get(DevicePtr pointer){
			return ImageViewBuilder(pointer);
		}

		ImageViewBuilder image(const VkImage& image){
			info.image = image;
			return *this;
		}

		ImageViewBuilder viewType(const VkImageViewType& type){
			info.viewType = type;
			return *this;
		}

		ImageViewBuilder format(const VkFormat& format){
			info.format = format;
			return *this;
		}

		ImageViewBuilder components(const VkComponentMapping& components){
			info.components = components;
			return *this;
		}

		ImageViewBuilder subresourceRange(const VkImageSubresourceRange& resource){
			info.subresourceRange = resource;
			return *this;
		}

		ImageViewPtr build(){
			VkImageView view = VK_NULL_HANDLE;
			auto err = vkCreateImageView(**logicalDevice,&info,VK_NULL_HANDLE,&view);
			Check(err);

			return std::make_shared<delete_wrap_ptr<VkImageView, DevicePtr>>(
				view,
				[](VkImageView ptr, DevicePtr device) {
					std::cout << STR(vkDestroyImageView) << std::endl;
					vkDestroyImageView(**device, ptr, VK_NULL_HANDLE);
				},
				logicalDevice);
		}

	};//ImageViewBuilder

	class ShaderModuleBuilder {
		VkShaderModuleCreateInfo info;
		DevicePtr logicalDevice;
	public:
		ShaderModuleBuilder(DevicePtr pointer) :logicalDevice(pointer) {
			info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			info.pNext = VK_NULL_HANDLE;
			info.flags = 0;
		}

		static ShaderModuleBuilder get(DevicePtr pointer) {
			return ShaderModuleBuilder(pointer);
		}

		ShaderModuleBuilder code(const Code& code) {
			info.codeSize = code.size();
			info.pCode = reinterpret_cast<const uint32_t*>(code.data());
			return *this;
		}

		ShaderModulePtr build() {
			VkShaderModule module = VK_NULL_HANDLE;
			auto err = vkCreateShaderModule(**logicalDevice, &info, VK_NULL_HANDLE, &module);
			Check(err);

			return std::make_shared<delete_wrap_ptr<VkShaderModule, DevicePtr>>(
				module,
				[](VkShaderModule ptr, DevicePtr device) {
					std::cout << STR(vkDestroyShaderModule) << std::endl;
					vkDestroyShaderModule(**device, ptr, VK_NULL_HANDLE);
				},
				logicalDevice);
		}
		
	};//ShaderModuleBuilder

	class DescriptorPoolBuilder{
		VkDescriptorPoolCreateInfo info;
		DevicePtr logicalDevice;
		std::vector<VkDescriptorPoolSize> poolSizeList;
	public:
		DescriptorPoolBuilder(DevicePtr pointer):logicalDevice(pointer){
			info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			info.pNext = VK_NULL_HANDLE;
			info.flags = 0;
		}

		static DescriptorPoolBuilder get(DevicePtr pointer){
			return DescriptorPoolBuilder(pointer);
		}

		DescriptorPoolBuilder maxSets(const uint32_t set){
			info.maxSets = set;
			return *this;
		}

		DescriptorPoolBuilder addPoolSize(const std::function<void(VkDescriptorPoolSize&)>& createFunc){
			add<VkDescriptorPoolSize>(poolSizeList,createFunc);
			return *this;
		}

		DescriptorPoolPtr build() {
			info.poolSizeCount = poolSizeList.size();
			info.pPoolSizes = poolSizeList.data();

			VkDescriptorPool pool = VK_NULL_HANDLE;
			auto err = vkCreateDescriptorPool(**logicalDevice, &info, VK_NULL_HANDLE, &pool);
			Check(err);

			return std::make_shared<delete_wrap_ptr<VkDescriptorPool, DevicePtr>>(
				pool,
				[](VkDescriptorPool ptr, DevicePtr device) {
					std::cout << STR(vkDestroyDescriptorPool) << std::endl;
					vkDestroyDescriptorPool(**device, ptr, VK_NULL_HANDLE);
				},
				logicalDevice);
		}

	};//DescriptorPoolBuilder

	class PipelineLayoutBuilder{
		VkPipelineLayoutCreateInfo info;
		DevicePtr logicalDevice;
		std::vector<VkDescriptorSetLayout> layoutList;
		std::vector<VkPushConstantRange> constantRangeList;
	public:
		PipelineLayoutBuilder(DevicePtr pointer):logicalDevice(pointer){
			info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			info.pNext = VK_NULL_HANDLE;
			info.flags = 0;
		}

		~PipelineLayoutBuilder(){}

		static PipelineLayoutBuilder get(DevicePtr pointer){
			return PipelineLayoutBuilder(pointer);
		}

		PipelineLayoutPtr build() {
			info.setLayoutCount = layoutList.size();
			if(!layoutList.empty())info.pSetLayouts = layoutList.data();

			info.pushConstantRangeCount = constantRangeList.size();
			if(!constantRangeList.empty())info.pPushConstantRanges = constantRangeList.data();

			VkPipelineLayout layout = VK_NULL_HANDLE;
			auto err = vkCreatePipelineLayout(**logicalDevice, &info, VK_NULL_HANDLE, &layout);
			Check(err);

			return std::make_shared<delete_wrap_ptr<VkPipelineLayout, DevicePtr>>(
				layout,
				[](VkPipelineLayout ptr, DevicePtr device) {
					std::cout << STR(vkDestroyPipelineLayout) << std::endl;
					vkDestroyPipelineLayout(**device, ptr, VK_NULL_HANDLE);
				},
				logicalDevice);
		}

		PipelineLayoutBuilder addPushConstantRange(const std::function<void(VkPushConstantRange&)> createFunc){
			add<VkPushConstantRange>(constantRangeList, createFunc);
			return *this;
		}

		PipelineLayoutBuilder addLayout(const std::function<void(VkDescriptorSetLayout&)> createFunc){
			add<VkDescriptorSetLayout>(layoutList, createFunc);
			return *this;
		}

	};//PiplineLayoutBuilder

	class GraphicsPipelineBuilder{
		VkGraphicsPipelineCreateInfo info;
		DevicePtr logicalDevice;
		std::vector<VkPipelineShaderStageCreateInfo> stageList;

		//vertexInputStateで使用 Enableフラグの代わりにも使用する。
		std::vector<VkVertexInputAttributeDescription> vertexAttributeDescriptionList;
		std::vector<VkVertexInputBindingDescription> vertexBindingDescriptionList;
		//viewportStateで使用 Enableフラグの代わりにも使用する。
		std::vector<VkViewport> viewportList;
		std::vector<VkRect2D> scissorList;
		//colorBlendStateで使用。
		std::vector<VkPipelineColorBlendAttachmentState> attachmentList;
		//dynamicStateで使用 Enableフラグの代わりにも使用する。
		std::vector<VkDynamicState> dynamicStateList;

		bool inputAssemblyStateEnable = false;
		bool tessellationStateEnable = false;
		bool rasterizationStateEnable = false;
		bool multisampleStateEnable = false;
		bool depthStencilStateEnable = false;
		bool colorBlendStateEnable = false;

		const std::string ENTRY_NAME = "main";

		VkPipelineVertexInputStateCreateInfo vertexInputState;
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyState;
		VkPipelineTessellationStateCreateInfo tessellationState;
		VkPipelineViewportStateCreateInfo viewportState;
		VkPipelineRasterizationStateCreateInfo rasterizationState;
		VkPipelineMultisampleStateCreateInfo multisampleState;
		VkPipelineDepthStencilStateCreateInfo depthStencilState;
		VkPipelineColorBlendStateCreateInfo colorBlendState;
		VkPipelineDynamicStateCreateInfo dynamicState;

		void addStage(VkShaderStageFlagBits stage, ShaderModulePtr& module){
			VkPipelineShaderStageCreateInfo sInfo = {};
			sInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			sInfo.pNext = VK_NULL_HANDLE;
			sInfo.flags = 0;
			sInfo.stage = stage;
			sInfo.module = **module;
			sInfo.pName = ENTRY_NAME.c_str();

			stageList.push_back(sInfo);
		}

	public:

		GraphicsPipelineBuilder(DevicePtr pointer):logicalDevice(pointer){
			info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			info.pNext = VK_NULL_HANDLE;
			info.flags = 0;
			info.basePipelineHandle = VK_NULL_HANDLE;
			info.basePipelineIndex = -1;

			vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			vertexInputState.pNext = NULL;
			vertexInputState.flags = 0;
			vertexInputState.vertexAttributeDescriptionCount = 0;
			vertexInputState.vertexBindingDescriptionCount = 0;
			vertexInputState.pVertexAttributeDescriptions = nullptr;
			vertexInputState.pVertexBindingDescriptions = nullptr;

			inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			inputAssemblyState.pNext = NULL;
			inputAssemblyState.flags = 0;

			tessellationState.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
			tessellationState.pNext = NULL;
			tessellationState.flags = 0;
			tessellationState.patchControlPoints = 0;

			viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			viewportState.pNext = NULL;
			viewportState.flags = 0;

			rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			rasterizationState.pNext = NULL;
			rasterizationState.flags = 0;

			multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			multisampleState.pNext = NULL;
			multisampleState.flags = 0;

			depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			depthStencilState.pNext = NULL;
			depthStencilState.flags = 0;

			colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			colorBlendState.pNext = NULL;
			colorBlendState.flags = 0;

			dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
			dynamicState.pNext = NULL;
			dynamicState.flags = 0;
		}

		static GraphicsPipelineBuilder get(DevicePtr pointer){
			return GraphicsPipelineBuilder(pointer);
		}

		GraphicsPipelineBuilder addVertexStage(ShaderModulePtr& module){
			addStage(VK_SHADER_STAGE_VERTEX_BIT,module);
			return *this;
		}

		GraphicsPipelineBuilder addTessellationControlStage(ShaderModulePtr& module){
			addStage(VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT ,module);
			return *this;
		}

		GraphicsPipelineBuilder addTessellationEvaluationStage(ShaderModulePtr& module){
			addStage(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT  ,module);
			return *this;
		}

		GraphicsPipelineBuilder addGeometryStage(ShaderModulePtr& module){
			addStage(VK_SHADER_STAGE_GEOMETRY_BIT,module);
			return *this;
		}

		GraphicsPipelineBuilder addFragmentStage(ShaderModulePtr& module){
			addStage(VK_SHADER_STAGE_FRAGMENT_BIT,module);
			return *this;
		}

		GraphicsPipelineBuilder addComputeStage(ShaderModulePtr& module){
			addStage(VK_SHADER_STAGE_COMPUTE_BIT,module);
			return *this;
		}

		GraphicsPipelineBuilder addAllGraphicsStage(ShaderModulePtr& module){
			addStage(VK_SHADER_STAGE_ALL_GRAPHICS,module);
			return *this;
		}

		GraphicsPipelineBuilder addAllStage(ShaderModulePtr& module){
			addStage(VK_SHADER_STAGE_ALL,module);
			return *this;
		}

		//GraphicsPipelineBuilder addRaygenStage(ShaderModulePtr& module){
		//	addStage(VK_SHADER_STAGE_RAYGEN_BIT_KHR,module);
		//	return *this;
		//}

		//GraphicsPipelineBuilder addAnyHitStage(ShaderModulePtr& module){
		//	addStage(VK_SHADER_STAGE_ANY_HIT_BIT_KHR,module);
		//	return *this;
		//}

		//GraphicsPipelineBuilder addClosestHitStage(ShaderModulePtr& module){
		//	addStage(VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR,module);
		//	return *this;
		//}

		//GraphicsPipelineBuilder addMissStage(ShaderModulePtr& module){
		//	addStage(VK_SHADER_STAGE_MISS_BIT_KHR,module);
		//	return *this;
		//}

		//GraphicsPipelineBuilder addIntersectionStage(ShaderModulePtr& module){
		//	addStage(VK_SHADER_STAGE_INTERSECTION_BIT_KHR,module);
		//	return *this;
		//}

		//GraphicsPipelineBuilder addCallableStage(ShaderModulePtr& module){
		//	addStage(VK_SHADER_STAGE_CALLABLE_BIT_KHR,module);
		//	return *this;
		//}

		GraphicsPipelineBuilder addTaskStage(ShaderModulePtr& module){
			addStage(VK_SHADER_STAGE_TASK_BIT_NV,module);
			return *this;
		}

		GraphicsPipelineBuilder addMeshStage(ShaderModulePtr& module){
			addStage(VK_SHADER_STAGE_MESH_BIT_NV,module);
			return *this;
		}

		GraphicsPipelineBuilder addVertexBindingDescription(const std::function<void(VkVertexInputBindingDescription&)>& createFunc){
			add<VkVertexInputBindingDescription>(vertexBindingDescriptionList, createFunc);
			return *this;
		}

		GraphicsPipelineBuilder addVertexAttributeDescription(const std::function<void(VkVertexInputAttributeDescription&)>& createFunc) {
			add<VkVertexInputAttributeDescription>(vertexAttributeDescriptionList, createFunc);
			return *this;
		}

		GraphicsPipelineBuilder topology(VkPrimitiveTopology topology){
			inputAssemblyState.topology = topology;
			inputAssemblyStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder primitiveRestartEnableOn(){
			inputAssemblyState.primitiveRestartEnable = VK_TRUE;
			inputAssemblyStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder primitiveRestartEnableOff() {
			inputAssemblyState.primitiveRestartEnable = VK_FALSE;
			inputAssemblyStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder patchControlPoints(const uint32_t points){
			tessellationState.patchControlPoints = points;
			tessellationStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder addViewports(const std::function<void(VkViewport&)> createFunc){
			add<VkViewport>(viewportList,createFunc);
			return *this;
		}

		GraphicsPipelineBuilder addScissors(const std::function<void(VkRect2D&)> createFunc){
			add<VkRect2D>(scissorList,createFunc);
			return *this;
		}

		GraphicsPipelineBuilder depthClampEnableOn(){
			rasterizationState.depthClampEnable = VK_TRUE;
			rasterizationStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder depthClampEnableOff(){
			rasterizationState.depthClampEnable = VK_FALSE;
			rasterizationStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder rasterizerDiscardEnableOn(){
			rasterizationState.rasterizerDiscardEnable = VK_TRUE;
			rasterizationStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder rasterizerDiscardEnableOff(){
			rasterizationState.rasterizerDiscardEnable = VK_FALSE;
			rasterizationStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder polygonModeFill(){
			rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
			rasterizationStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder polygonModeLine() {
			rasterizationState.polygonMode = VK_POLYGON_MODE_LINE;
			rasterizationStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder polygonModePoint() {
			rasterizationState.polygonMode = VK_POLYGON_MODE_POINT;
			rasterizationStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder polygonModeFillRectangleNV() {
			rasterizationState.polygonMode = VK_POLYGON_MODE_FILL_RECTANGLE_NV;
			rasterizationStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder cullModeNone(){
			rasterizationState.cullMode = VK_CULL_MODE_NONE;
			rasterizationStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder cullModeFront() {
			rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
			rasterizationStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder cullModeBack() {
			rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
			rasterizationStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder cullModeFrontAndBack() {
			rasterizationState.cullMode = VK_CULL_MODE_FRONT_AND_BACK;
			rasterizationStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder frontFaceCounterClockWise(){
			rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
			rasterizationStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder frontFaceClockWise(){
			rasterizationState.frontFace = VK_FRONT_FACE_CLOCKWISE;
			rasterizationStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder depthBiasEnableOn(){
			rasterizationState.depthBiasEnable = VK_TRUE;
			rasterizationStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder depthBiasEnableOff(){
			rasterizationState.depthBiasEnable = VK_FALSE;
			rasterizationStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder depthBiasConstantFactor(const float& value){
			rasterizationState.depthBiasConstantFactor = value;
			rasterizationStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder depthBiasClamp(const float& value){
			rasterizationState.depthBiasClamp = value;
			rasterizationStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder depthBiasSlopeFactor(const float& value){
			rasterizationState.depthBiasSlopeFactor = value;
			rasterizationStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder lineWidth(const float& value){
			rasterizationState.lineWidth = value;
			rasterizationStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder rasterizationSampleCount1bit(){
			multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
			multisampleStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder rasterizationSampleCount2bit(){
			multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_2_BIT;
			multisampleStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder rasterizationSampleCount4bit(){
			multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_4_BIT;
			multisampleStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder rasterizationSampleCount8bit(){
			multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_8_BIT;
			multisampleStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder rasterizationSampleCount16bit(){
			multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_16_BIT;
			multisampleStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder rasterizationSampleCount32bit(){
			multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_32_BIT;
			multisampleStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder rasterizationSampleCount64bit(){
			multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_64_BIT;
			multisampleStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder sampleShadingEnableOn(){
			multisampleState.sampleShadingEnable = VK_TRUE;
			multisampleStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder sampleShadingEnableOff(){
			multisampleState.sampleShadingEnable = VK_FALSE;
			multisampleStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder minSampleShading(const float value){
			multisampleState.minSampleShading = value;
			multisampleStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder pSampleMask(const VkSampleMask* value){
			multisampleState.pSampleMask = value;
			multisampleStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder alphaToCoverageEnableOn(){
			multisampleState.alphaToCoverageEnable = VK_TRUE;
			multisampleStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder alphaToCoverageEnableOff(){
			multisampleState.alphaToCoverageEnable = VK_FALSE;
			multisampleStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder alphaToOneEnableOn(){
			multisampleState.alphaToOneEnable = VK_TRUE;
			multisampleStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder alphaToOneEnableOff(){
			multisampleState.alphaToOneEnable = VK_FALSE;
			multisampleStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder depthTestEnableOn(){
			depthStencilState.depthTestEnable = VK_TRUE;
			depthStencilStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder depthTestEnableOff(){
			depthStencilState.depthTestEnable = VK_FALSE;
			depthStencilStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder depthWriteEnableOn(){
			depthStencilState.depthWriteEnable = VK_TRUE;
			depthStencilStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder depthWriteEnableOff(){
			depthStencilState.depthWriteEnable = VK_FALSE;
			depthStencilStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder depthCompareOpNever(){
			depthStencilState.depthCompareOp = VK_COMPARE_OP_NEVER;
			depthStencilStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder depthCompareOpLess(){
			depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
			depthStencilStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder depthCompareOpEqual(){
			depthStencilState.depthCompareOp = VK_COMPARE_OP_EQUAL;
			depthStencilStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder depthCompareOpLessOrEqual(){
			depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
			depthStencilStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder depthCompareOpGreater(){
			depthStencilState.depthCompareOp = VK_COMPARE_OP_GREATER;
			depthStencilStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder depthCompareOpNotEqual(){
			depthStencilState.depthCompareOp = VK_COMPARE_OP_NOT_EQUAL;
			depthStencilStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder depthCompareOpGreaterOrEqual(){
			depthStencilState.depthCompareOp = VK_COMPARE_OP_GREATER_OR_EQUAL ;
			depthStencilStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder depthCompareOpAlways(){
			depthStencilState.depthCompareOp = VK_COMPARE_OP_ALWAYS;
			depthStencilStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder depthBoundsTestEnableOn(){
			depthStencilState.depthBoundsTestEnable = VK_TRUE;
			depthStencilStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder depthBoundsTestEnableOff(){
			depthStencilState.depthBoundsTestEnable = VK_FALSE;
			depthStencilStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder stencilTestEnableOn(){
			depthStencilState.stencilTestEnable = VK_TRUE;
			depthStencilStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder stencilTestEnableOff(){
			depthStencilState.stencilTestEnable = VK_FALSE;
			depthStencilStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder stencilFrontState(const VkStencilOpState& state){
			depthStencilState.front = state;
			depthStencilStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder stencilBackState(const VkStencilOpState& state){
			depthStencilState.back = state;
			depthStencilStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder minDepthBounds(const float& value){
			depthStencilState.minDepthBounds = value;
			depthStencilStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder maxDepthBounds(const float& value){
			depthStencilState.maxDepthBounds = value;
			depthStencilStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder logicOpEnableOn(){
			colorBlendState.logicOpEnable = VK_TRUE;
			colorBlendStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder logicOpEnableOff(){
			colorBlendState.logicOpEnable = VK_FALSE;
			colorBlendStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder logicOpStateClear(){
			colorBlendState.logicOp = VK_LOGIC_OP_CLEAR;
			colorBlendStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder logicOpStateAnd(){
			colorBlendState.logicOp = VK_LOGIC_OP_AND;
			colorBlendStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder logicOpStateReverse(){
			colorBlendState.logicOp = VK_LOGIC_OP_AND_REVERSE;
			colorBlendStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder logicOpStateCopy(){
			colorBlendState.logicOp = VK_LOGIC_OP_COPY;
			colorBlendStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder logicOpStateInverted(){
			colorBlendState.logicOp = VK_LOGIC_OP_AND_INVERTED;
			colorBlendStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder logicOpStateNoOp(){
			colorBlendState.logicOp = VK_LOGIC_OP_NO_OP;
			colorBlendStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder logicOpStateXor(){
			colorBlendState.logicOp = VK_LOGIC_OP_XOR;
			colorBlendStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder logicOpStateOr(){
			colorBlendState.logicOp = VK_LOGIC_OP_OR;
			colorBlendStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder logicOpStateNor(){
			colorBlendState.logicOp = VK_LOGIC_OP_NOR;
			colorBlendStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder logicOpStateEquivalent(){
			colorBlendState.logicOp = VK_LOGIC_OP_EQUIVALENT;
			colorBlendStateEnable = true;
			return *this;
		}
		

		GraphicsPipelineBuilder logicOpStateInvert(){
			colorBlendState.logicOp = VK_LOGIC_OP_INVERT;
			colorBlendStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder logicOpStateOrReverse(){
			colorBlendState.logicOp = VK_LOGIC_OP_OR_REVERSE;
			colorBlendStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder logicOpStateCopyInverted(){
			colorBlendState.logicOp = VK_LOGIC_OP_COPY_INVERTED;
			colorBlendStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder logicOpStateOrInverted(){
			colorBlendState.logicOp = VK_LOGIC_OP_OR_INVERTED;
			colorBlendStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder logicOpStateNand(){
			colorBlendState.logicOp = VK_LOGIC_OP_NAND;
			colorBlendStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder logicOpStateSet(){
			colorBlendState.logicOp = VK_LOGIC_OP_SET;
			colorBlendStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder addAttachment(const std::function<void(VkPipelineColorBlendAttachmentState&)>& createFunc){
			add<VkPipelineColorBlendAttachmentState>(attachmentList,createFunc);
			colorBlendStateEnable = true;
			return *this;
		}

		template<uint32_t idx>
		GraphicsPipelineBuilder blendConstants(const float& value){
			colorBlendState.blendConstants[idx] = value;
			colorBlendStateEnable = true;
			return *this;
		}

		GraphicsPipelineBuilder addDynamicState(const VkDynamicState& value){
			dynamicStateList.push_back(value);
			return *this;
		}

		GraphicsPipelineBuilder layout(PipelineLayoutPtr layout){
			info.layout = **layout;
			return *this;
		}

		GraphicsPipelineBuilder renderpass(RenderpassPtr renderpass){
			info.renderPass = **renderpass;
			return *this;
		}

		GraphicsPipelineBuilder subpass(const uint32_t& subpass){
			info.subpass = subpass;
			return *this;
		}

		GraphicsPipelineBuilder basePipelineHandle(PipelinePtr pipeline){
			info.basePipelineHandle = **pipeline;
			return *this;
		}

		GraphicsPipelineBuilder basePipelineIndex(const int32_t& index){
			info.basePipelineIndex = index;
			return *this;
		}

		PipelinePtr build(VkPipelineCache cache){

			assert(colorBlendStateEnable
				&& inputAssemblyStateEnable
				&& tessellationStateEnable
				&& rasterizationStateEnable
				&& multisampleStateEnable
				&& !scissorList.empty()
				&& !viewportList.empty()
				&& !stageList.empty());

			if (!stageList.empty()) {
				info.stageCount = stageList.size();
				info.pStages = stageList.data();
			}
			if(!vertexAttributeDescriptionList.empty()){
				vertexInputState.vertexAttributeDescriptionCount = vertexAttributeDescriptionList.size();
				vertexInputState.pVertexAttributeDescriptions = vertexAttributeDescriptionList.data();
			}
			if(!vertexBindingDescriptionList.empty()){
				vertexInputState.vertexBindingDescriptionCount = vertexBindingDescriptionList.size();
				vertexInputState.pVertexBindingDescriptions = vertexBindingDescriptionList.data();
			}
			if (!viewportList.empty()) {
				viewportState.viewportCount = viewportList.size();
				viewportState.pViewports = viewportList.data();
			}
			if (!scissorList.empty()) {
				viewportState.scissorCount = scissorList.size();
				viewportState.pScissors = scissorList.data();
			}
			if (!attachmentList.empty()) {
				colorBlendState.attachmentCount = attachmentList.size();
				colorBlendState.pAttachments = attachmentList.data();
			}

			if (!dynamicStateList.empty()) { //Optional
				dynamicState.dynamicStateCount = dynamicStateList.size();
				dynamicState.pDynamicStates = dynamicStateList.data();
				info.pDynamicState = &dynamicState;
			}
			if (depthStencilStateEnable) //Optional
				info.pDepthStencilState = &depthStencilState;
			
			info.pVertexInputState = &vertexInputState;
			info.pViewportState = &viewportState;
			info.pColorBlendState = &colorBlendState;
			info.pInputAssemblyState = &inputAssemblyState;
			info.pTessellationState = &tessellationState;
			info.pRasterizationState = &rasterizationState;
			info.pMultisampleState = &multisampleState;

			VkPipeline pipeline = VK_NULL_HANDLE;
			auto err = vkCreateGraphicsPipelines(**logicalDevice, cache, 1, &info, VK_NULL_HANDLE, &pipeline);
			Check(err);

			return std::make_shared<delete_wrap_ptr<VkPipeline, DevicePtr>>(
				pipeline,
				[](VkPipeline ptr, DevicePtr device) {
					std::cout << STR(vkDestroyPipeline) << std::endl;
					vkDestroyPipeline(**device, ptr, VK_NULL_HANDLE);
				},
				logicalDevice);
		}

	};

	class FrameBufferBuilder {
		VkFramebufferCreateInfo info;
		DevicePtr logicalDevice;
		std::vector<VkImageView> attachmentList;
	public:
		FrameBufferBuilder(DevicePtr pointer) : logicalDevice(pointer){
			info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			info.pNext = VK_NULL_HANDLE;
			info.flags = 0;

			info.attachmentCount = 0;
			info.pAttachments = nullptr;
		}

		~FrameBufferBuilder(){}

		static FrameBufferBuilder get(DevicePtr pointer){
			return FrameBufferBuilder(pointer);
		}

		FrameBufferBuilder renderPass(RenderpassPtr pointer){
			info.renderPass = **pointer;
			return *this;
		}

		FrameBufferBuilder width(const uint32_t width){
			info.width = width;
			return *this;
		}

		FrameBufferBuilder height(const uint32_t height){
			info.height = height;
			return *this;
		}

		FrameBufferBuilder layers(const uint32_t layers){
			info.layers = layers;
			return *this;
		}

		FrameBufferBuilder addAttachment(ImageViewPtr& pointer){
			attachmentList.push_back(**pointer);
			return *this;
		}

		FramebufferPtr build() {
			info.attachmentCount = attachmentList.size();
			if(!attachmentList.empty())info.pAttachments = attachmentList.data();

			VkFramebuffer buffer = VK_NULL_HANDLE;
			auto err = vkCreateFramebuffer(**logicalDevice, &info, VK_NULL_HANDLE, &buffer);
			Check(err);

			return std::make_shared<delete_wrap_ptr<VkFramebuffer, DevicePtr>>(
				buffer,
				[](VkFramebuffer ptr, DevicePtr device) {
					std::cout << STR(vkDestroyFramebuffer) << std::endl;
					vkDestroyFramebuffer(**device, ptr, VK_NULL_HANDLE);
				},
				logicalDevice);
		}

	};

}