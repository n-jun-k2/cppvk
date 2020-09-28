#pragma once

#pragma warning(push)
#pragma warning(disable : 26812)
#pragma warning(disable : 4505)

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

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
	 * @brief 生成・破壊を制御する為オブジェクト
	 * 
	 * @tparam F 破壊処理のシグネチャ
	 * @tparam Args 破壊処理の対象・依存オブジェクト
	 */
	template<class F,  class... Args> 
		class raii {
			F deleteFunc;
		protected:
			std::tuple<Args...> args;
		public:
			raii(F f, Args... a) : deleteFunc(f), args(a...) {}
			virtual ~raii(){
				if(deleteFunc) std::apply(deleteFunc, args); 
			}
			auto& get() const noexcept{
				return std::get<0>(args);
			}
			auto&  operator*() const noexcept {
				return get();
			}
		};

	template <typename T>
	struct identity {using type=T;};

	struct PhyscialDeivceSet;

	using ExtensionPropertiesList = std::vector< VkExtensionProperties>;
	using LayerPropertiesList = std::vector< VkLayerProperties>;
	using PhysicalDeviceList = std::vector< VkPhysicalDevice>;
	using PhysicalDeviceGroupList = std::vector< VkPhysicalDeviceGroupProperties>;
	using Names = std::vector<const char*>;
	using Indexs = std::vector<uint32_t>;
	using Priorities = std::vector<float>;
	using Code = std::vector<char>;
	using QueueCreateInfos = std::vector<VkDeviceQueueCreateInfo>;
	using SurfaceFormats = std::vector<VkSurfaceFormatKHR>;
	using PresentModes = std::vector<VkPresentModeKHR>;
	using PhysicalDeviceQueueProps = std::vector<VkQueueFamilyProperties>;
	using ChoosePhysicalDeviceFuncInstanceBuilder = std::function<bool (PhyscialDeivceSet&)>;
	using IsSuitableQueuePropFuncInstanceBuilder = std::function<bool(VkQueueFamilyProperties)>;
	template <typename T>using shared_pointer = std::shared_ptr<std::remove_pointer_t<T>>;

	#define TO_VKPFN(NAME) PFN_##NAME
	#define GetVkInstanceProcAddr(instance, FUNCNAME) \
		(TO_VKPFN(FUNCNAME))vkGetInstanceProcAddr(instance, #FUNCNAME)


	template<class T, template<class cT, class A = std::allocator<cT>>class Container>
	void _add(Container<T>& container, const std::function<void(T&)>& createFunc) {
		auto item = T{};
		createFunc(item);
		container.push_back(item);
	}
	/* VkResult判定処理	 */
	void Check(const VkResult& result, const std::string& message = "")
	{
		if (result == VK_SUCCESS)return;
		std::cerr << "VkResult : " << result << std::endl;
		throw std::runtime_error(message);
	}
	/* target文字列配列が全てsource配列の要素にあるのかの存在を確認する。存在した場合：真*/
	template<class _T, template<class... Args>class Container>
	static bool _ExistSupport(const Names& target, const Container<_T>& source, std::function<const char* (const _T&)> toString) noexcept {
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
	/* source配列にtarget名が全て存在するか。 */
	static bool ExistSupport(const Names& target, const ExtensionPropertiesList& source) {
		return _ExistSupport<VkExtensionProperties, std::vector>(target, source, [](VkExtensionProperties prop) {return prop.extensionName; });
	}
	/* source配列にtarget名が全て存在するか。 */
	static bool ExistSupport(const Names& target, const LayerPropertiesList& source) {
		return _ExistSupport<VkLayerProperties, std::vector>(target, source, [](VkLayerProperties prop) {return prop.layerName; });
	}
	/*ファイル読み込み処理*/
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
	/*VkAttachmentDescriptionを作成するサンプル*/
	static VkAttachmentDescription AttachmentDescriptionCreate(const VkFormat format, const VkSampleCountFlagBits sample_bit){
		auto attachment = VkAttachmentDescription{};
		/*!@  カラーバッファのアタッチメント情報を設定*/
		attachment.format = format;
		attachment.samples = sample_bit;
		/*!@ 色・深さデータ設定*/
		attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		/*!@ ステンシルバッファのデータ設定*/
		attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		/*!@
		Vulkan APIのテクスチャとフレームバッファは、
		VkImageという特定のピクセルフォーマットを持つオブジェクトによって表します。が
		メモリ内の｛ピクセルのレイアウト｝は画像に対して何を使用としているかによって変わることがあります。
		*/
		attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		return attachment;
	}
	/*VkSubpassDependencyを作成するサンプル*/
	static VkSubpassDependency SubpassDependencyCreate(){
		auto dependency = VkSubpassDependency{};
		/*!@依存関係・依存サブパスのインデックスを指定*/
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		/*!@待機操作、操作発生イベントを指定する。*/
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		/*!@カラーアタッチメント出力設定*/
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask =
			VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		return dependency;
	}
	/*VkSubpassDescriptionを作成するサンプル*/
	static VkSubpassDescription SubpassDescriptionCreate(const VkAttachmentReference& colorAttachment, const VkAttachmentReference& depthAttachment){
		auto description = VkSubpassDescription{};
		description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		description.colorAttachmentCount = 1;
		description.pColorAttachments = &colorAttachment;
		description.pDepthStencilAttachment = &depthAttachment;
		return description;
	}

	/**
	* @brief Settings used in the swap chain
	*/
	struct SwapchainSupportDetails {
		VkSurfaceCapabilitiesKHR surfaceCapabilities;
		SurfaceFormats surfaceFormats;
		PresentModes presentModes;
		bool IsComplate() {
			return !surfaceFormats.empty() && !presentModes.empty();
		}
		/*サーフェイスの優先すべきフォーマット情報を返す。*/
		VkSurfaceFormatKHR chooseSwapSurfaceFormat() {
			//サーフィス情報に優先フォーマットがない場合
			if (surfaceFormats.size() == 1 && surfaceFormats[0].format == VK_FORMAT_UNDEFINED)
				return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

			//フォーマットに縛りがある場合はリストを探索し、希望の組み合わせが可能かを確認.
			for (const auto& availableFormat : surfaceFormats)
			{
				if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM &&
					availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
					return availableFormat;
			}

			return surfaceFormats[0];
		}
		/*プレゼントモードの優先すべきモードを返す。*/
		VkPresentModeKHR chooseSwapPresentMode()
		{
			for (const auto& present : presentModes)
			{
				if (present == VK_PRESENT_MODE_MAILBOX_KHR)return present;
			}
			return VK_PRESENT_MODE_FIFO_KHR;
		}
		/*範囲データが異常値の場合空を返す。*/
		VkExtent2D chooseSwapExtent() {
			if (surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
				return surfaceCapabilities.currentExtent;

			VkExtent2D extent2d = {};
			return extent2d;
		}
	};

	/* 物理デバイス情報セット	*/
	struct PhyscialDeivceSet {
		VkPhysicalDevice device;
		VkPhysicalDeviceProperties props;
		VkPhysicalDeviceFeatures features;
		VkPhysicalDeviceMemoryProperties memory;
		cppvk::ExtensionPropertiesList extensions;
		cppvk::LayerPropertiesList validations;
		cppvk::PhysicalDeviceQueueProps qprops;

		PhyscialDeivceSet(VkPhysicalDevice gpu) :device(gpu) {
			vkGetPhysicalDeviceProperties(gpu, &props);
			vkGetPhysicalDeviceFeatures(gpu, &features);
			vkGetPhysicalDeviceMemoryProperties(gpu, &memory);
			extensions = GetEnumeratePhysicalDeviceExtensions(gpu);
			validations = GetEnumerateDeviceLayer(gpu);
			qprops = GetEnumeratePhysicalDeviceQueueProps(gpu);
		}
		/*物理デバイスのキューファミリ一覧

			デバイスキューは、CPUから書き込んだコマンドをGPUへ流すパイプのようなもの。
			Vulkanでは、処理ごとに区分けされています。

			VK_QUEUE_GRAPHICS_BIT グラフィックス処理⽤のキュー
			VK_QUEUE_COMPUTE_BIT コンピュート処理⽤のキュー
			VK_QUEUE_TRANSFER_BIT データ転送処理専⽤のキュー
			VK_QUEUE_SPARSE_BINDING_BIT スパースメモリ管理処理のキュー
			VK_QUEUE_PROTECTED_BIT 保護メモリ機能が有効なときに使⽤するキュー
		*/
		PhysicalDeviceQueueProps GetEnumeratePhysicalDeviceQueueProps(const VkPhysicalDevice gpu) {
			assert(gpu != VK_NULL_HANDLE);
			uint32_t  count = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(gpu, &count, nullptr);
			PhysicalDeviceQueueProps queue(count);
			vkGetPhysicalDeviceQueueFamilyProperties(gpu, &count, queue.data());
			return queue;
		}
		/*物理デバイスの拡張機能一覧*/
		ExtensionPropertiesList GetEnumeratePhysicalDeviceExtensions(const VkPhysicalDevice gpu) {
			assert(gpu != VK_NULL_HANDLE);
			uint32_t size = 0;
			Check(vkEnumerateDeviceExtensionProperties(gpu, nullptr, &size, nullptr));
			ExtensionPropertiesList list(size);
			Check(vkEnumerateDeviceExtensionProperties(gpu, nullptr, &size, list.data()));
			return list;
		}
		/*物理デバイスの検証機能一覧*/
		LayerPropertiesList GetEnumerateDeviceLayer(const VkPhysicalDevice gpu) {
			assert(gpu != VK_NULL_HANDLE);
			uint32_t size = 0;
			Check(vkEnumerateDeviceLayerProperties(gpu, &size, nullptr));
			LayerPropertiesList list(size);
			Check(vkEnumerateDeviceLayerProperties(gpu, &size, list.data()));
			return list;
		}

	};

	/* delete_warp_ptr を makeする為 templateで取得し*/
	template<class Base>
	class _Instance : public Base {
	public:
		using Base::Base;
		PhysicalDeviceList GetEnumeratePhysicalDevices()
		{
			assert(**this != VK_NULL_HANDLE);
			uint32_t size;
			Check(vkEnumeratePhysicalDevices(**this, &size, nullptr));
			PhysicalDeviceList list(size);
			Check(vkEnumeratePhysicalDevices(**this, &size, list.data()));
			return list;
		}
		PhysicalDeviceGroupList GetEnumeratePhysicalDeviceGroupsKHR() {
			assert(**this != VK_NULL_HANDLE);
			uint32_t size = 0;
			Check(vkEnumeratePhysicalDeviceGroupsKHR(**this, &size, nullptr));
			PhysicalDeviceGroupList list(size);
			Check(vkEnumeratePhysicalDeviceGroupsKHR(**this, &size, list.data()));
			return list;
		}
		PhysicalDeviceGroupList GetEnumeratePhysicalDeviceGroups() {
			assert(**this != VK_NULL_HANDLE);
			uint32_t size = 0;
			Check(vkEnumeratePhysicalDeviceGroups(**this, &size, nullptr));
			PhysicalDeviceGroupList list(size);
			Check(vkEnumeratePhysicalDeviceGroups(**this, &size, list.data()));
			return list;
		}
		/*削除処理をする必要がない為そのままデバイスポインタを返す。*/
		PhyscialDeivceSet ChooseGpu(ChoosePhysicalDeviceFuncInstanceBuilder isSuitable) {
	
			PhysicalDeviceList gpus = GetEnumeratePhysicalDevices();
			if(gpus.empty())
				throw std::runtime_error("failed to find GPUs with vulkan support!");

			for (auto&& gpu : gpus) {
				auto dev_set = PhyscialDeivceSet(gpu);

				/*最適条件ではない場合繰り返す。*/
				if (!isSuitable(dev_set))	continue;

				return PhyscialDeivceSet(gpu);
			}
			return VK_NULL_HANDLE;
		}

		PFN_vkCreateDebugReportCallbackEXT GetCreateDebugReportCallbackEXT(){
			return GetVkInstanceProcAddr(**this, vkCreateDebugReportCallbackEXT);
		}

		PFN_vkDebugReportMessageEXT GetDebugReportMessageEXT(){
			return GetVkInstanceProcAddr(**this, vkDebugReportMessageEXT);
		}

		PFN_vkDestroyDebugReportCallbackEXT GetkDestroyDebugReportCallbackEXT(){
			return GetVkInstanceProcAddr(**this, vkDestroyDebugReportCallbackEXT);
		}
	};
	using Instance = _Instance< raii < typename identity<std::function<void(VkInstance)>>::type, VkInstance>>;
	using InstancePtr = shared_pointer <Instance>;

	template<class Base>
	class _Surface : public Base {
	public:
		using  Base::Base;
		/*
		サーフェイスの能力値の取得
		*/
		VkSurfaceCapabilitiesKHR GetSurfaceCapabilities(VkPhysicalDevice gpu)
		{
			VkSurfaceCapabilitiesKHR temp{};
			Check(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, **this, &temp));
			return temp;
		}
		/*
		物理デバイスのサーフェイスが利用可能な表示用フォーマット
		*/
		SurfaceFormats GetEnumerateSurfaceFormats(VkPhysicalDevice gpu)
		{
			uint32_t fcount;
			vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, **this, &fcount, nullptr);
			SurfaceFormats formats(fcount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, **this, &fcount, formats.data());
			return formats;
		}
		/*
		物理デバイスのサーフェイスが利用可能なプレゼントモード
		表・裏画面の切り替え操作(Presentという)を、どのような方式で行うか調べる。
		*/
		PresentModes GetEnumerateSurfacePresentmodes(VkPhysicalDevice gpu)
		{
			uint32_t pcount;
			vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, **this, &pcount, nullptr);
			PresentModes presentModes(pcount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, **this, &pcount, presentModes.data());
			return presentModes;
		}
		/*このデバイスのサーフェイス機能がサポートされているのか確認。*/
		bool GetPhysicalDevicceSurfaceSupportKHR(VkPhysicalDevice gpu, const uint32_t index) {
			VkBool32 isSupport = false;
			Check(vkGetPhysicalDeviceSurfaceSupportKHR(gpu, index, **this, &isSupport));
			return isSupport == VK_TRUE;
		}
	};
	using Surface = _Surface<raii<typename identity<std::function<void(VkSurfaceKHR, InstancePtr)>>::type, VkSurfaceKHR, InstancePtr>>;
	using SurfacePtr = shared_pointer <Surface>;

	template<class Base> class _Device;
	using Device = _Device<raii<typename identity<std::function<void(VkDevice)>>::type, VkDevice>>;
	using DevicePtr = shared_pointer<Device>;

	template<class Base>
	class _Fence : Base {
		public:
			using Base::Base;
	};
	using Fence = _Fence<raii<typename identity<std::function<void(VkFence, DevicePtr)>>::type, VkFence, DevicePtr>>;
	using FencePtr = shared_pointer<Fence>;

	template<class Base>
	class _Event : Base {
		public:
			using Base::Base;
	};
	using Event = _Event<raii<typename identity<std::function<void(VkEvent, DevicePtr)>>::type, VkEvent, DevicePtr>>;
	using EventPtr = shared_pointer<Event>;

	template<class Base>
	class _Semaphore : Base {
		public:
			using Base::Base;
	};
	using Semaphore = _Semaphore<raii<typename identity<std::function<void(VkSemaphore, DevicePtr)>>::type, VkSemaphore, DevicePtr>>;
	using SemaphorePtr = shared_pointer<Semaphore>;

	template<class Base> 
	class _Queue : Base{
	public:
		using Base::Base;
		template<
			template<class T, class A = std::allocator<T>> 
				class Container>
		void Submit(const VkPipelineStageFlags& stageFlag,
					FencePtr pfence,
					Container<VkSemaphore>& wait_semaphore_list, 
					Container<VkCommandBuffer>& cmdbuffer_list, 
					Container<VkSemaphore>& signal_semaphore_list){
			VkSubmitInfo info{};
			info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			info.pNext = VK_NULL_HANDLE;
			info.waitSemaphoreCount = static_cast<uint32_t>(wait_semaphore_list.size());
			info.pWaitSemaphores = wait_semaphore_list.data();
			info.commandBufferCount = static_cast<uint32_t>(cmdbuffer_list.size());
			info.pCommandBuffers = cmdbuffer_list.data();
			info.signalSemaphoreCount = static_cast<uint32_t>(signal_semaphore_list.size());
			info.pSignalSemaphores = signal_semaphore_list.data();
			info.pWaitDstStageMask = &stageFlag;
			// Todo:要見直し
			Check(vkQueueSubmit(**this, 1, &info, **pfence));
		}

		template<
			template<class T, class A = std::allocator<T>> 
				class Container>
		Container<VkResult> Present(Container<VkSemaphore>& semaphore_list, Container<VkSwapchainKHR>& swapchain_list, Container<uint32_t>& indice_list){
			VkPresentInfoKHR info{};
			info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			info.pNext = VK_NULL_HANDLE;
			info.waitSemaphoreCount = static_cast<uint32_t>(semaphore_list.size());
			info.pWaitSemaphores = semaphore_list.data();

			info.swapchainCount = static_cast<uint32_t>(swapchain_list.size());

			if(info.swapchainCount == indice_list.size()){
				std::runtime_error("The size of indice_list and swapchain_list must match.");
			}
			info.pSwapchains = swapchain_list.data();
			info.pImageIndices = indice_list.data();
			auto result_list = Container<VkResult>(info.swapchainCount);
			info.pResults = result_list.data();
			Check(vkQueuePresentKHR(**this, &info));
			return result_list;
		}
	};
	using Queue = _Queue<raii<typename identity<std::function<void(VkQueue, DevicePtr)>>::type, VkQueue, DevicePtr>>;
	using QueuePtr = shared_pointer<Queue>;

	template<class Base>
	class _Device : public Base {
	public:
		using Base::Base;

		QueuePtr getQueue(DevicePtr pointer, const uint32_t& family,const uint32_t& index) {
			VkQueue queue;
			vkGetDeviceQueue(**this, family, index, &queue);
			return std::make_shared<Queue>(
				[](VkQueue ptr, DevicePtr logicalDevice){
					std::cout << "Destory Queue" << std::endl;
				},queue,pointer
			);
		}
	}; 
	using Device = _Device<raii<typename identity<std::function<void(VkDevice)>>::type, VkDevice>>;
	using DevicePtr = shared_pointer<Device>;

	using Pipeline = raii<typename identity<std::function<void(VkPipeline, DevicePtr)>>::type, VkPipeline, DevicePtr>;
	using PipelinePtr = shared_pointer<Pipeline>;

	using Framebuffer = raii<typename identity<std::function<void(VkFramebuffer, DevicePtr)>>::type, VkFramebuffer, DevicePtr>;
	using FramebufferPtr = shared_pointer<Framebuffer>;

	using PipelineLayout = raii<typename identity<std::function<void(VkPipelineLayout, DevicePtr)>>::type, VkPipelineLayout, DevicePtr>;
	using PipelineLayoutPtr = shared_pointer<PipelineLayout>;

	using ShaderModule = raii<typename identity<std::function<void(VkShaderModule, DevicePtr)>>::type, VkShaderModule, DevicePtr>;
	using ShaderModulePtr = shared_pointer<ShaderModule>;

	using DescriptorPool = raii<typename identity<std::function<void(VkDescriptorPool, DevicePtr)>>::type, VkDescriptorPool, DevicePtr>;
	using DescriptorPoolPtr = shared_pointer<DescriptorPool>;

	using DebugMessanger = raii<typename identity<std::function<void(VkDebugUtilsMessengerEXT, InstancePtr)>>::type, VkDebugUtilsMessengerEXT, InstancePtr>;
	using DebugMessengerPtr = shared_pointer <DebugMessanger>;

	using ImageView = raii<typename identity<std::function<void(VkImageView, DevicePtr)>>::type, VkImageView, DevicePtr>;
	using ImageViewPtr = shared_pointer<ImageView> ;

	template<class Base>
	class _Image : public Base {
	public:
		using Base::Base;
		VkMemoryRequirements GetMemoryRequirements() {
			return std::apply([](VkImage image, DevicePtr device) {
				VkMemoryRequirements info = {};
				vkGetImageMemoryRequirements(**device, image, &info);
				return info;
			}, Base::args);
		}
	};
	using Image = _Image<raii<typename identity<std::function<void(VkImage, DevicePtr)>>::type,VkImage, DevicePtr>>;
	using ImagePtr = shared_pointer<Image>;

	template <class Base>
	class _Renderpass : public Base {
	public:
		using Base::Base;
	};
	using Renderpass = _Renderpass<raii<typename identity<std::function<void(VkRenderPass, DevicePtr)>>::type, VkRenderPass, DevicePtr>>;
	using RenderpassPtr = shared_pointer<Renderpass>;

	template<class Base>
	class _CommandPool : public Base {
	public:
		using Base::Base;
		void Trim(VkCommandPoolTrimFlags flags) {
			std::apply([&flags](VkCommandPool pool, DevicePtr device) {
				vkTrimCommandPool(**device, pool, flags);
			}, Base::args);
		}
		void Reset(VkCommandPoolResetFlags flags) {
			std::apply([&flags](VkCommandPool pool, DevicePtr device) {
				auto err = vkResetCommandPool(**device, pool, flags);
				Check(err); 
			}, Base::args);
		}
	};	
	using CommandPool = _CommandPool <raii<typename identity<std::function<void(VkCommandPool, DevicePtr)>>::type, VkCommandPool, DevicePtr>>;
	using CommandPoolPtr = shared_pointer<CommandPool>;

	template<class Base>
	class _CommandBuffer : public Base {
	public:
		using Base::Base;
	};
	using CommandBuffer = _CommandBuffer<raii<typename identity<std::function<void(VkCommandBuffer, DevicePtr)>>::type,VkCommandBuffer, DevicePtr>>;
	using CommandBufferPtr = shared_pointer<CommandBuffer>;

	template<class Base>
	class _Swapchain : Base{
	public:
		using Base::Base;
		std::vector<ImagePtr>GetImages() {
			return std::apply([](VkSwapchainKHR swapchain, DevicePtr device) {
				std::vector< VkImage> images;
				uint32_t count;

				vkGetSwapchainImagesKHR(**device, swapchain, &count, nullptr);
				images.resize(count);
				vkGetSwapchainImagesKHR(**device, swapchain, &count, images.data());

				std::vector<ImagePtr> s_images;
				s_images.resize(count);
				for(size_t i = 0; i < count; ++i) {
					//スワップチェーンが破棄されると自動的にクリーンアップされる為、クリーンアップコードを追加する必要はありません。
					s_images[i] = std::make_shared<Image>(nullptr, images[i], device);
				}
				return s_images;
			}, Base::args);
		}

		/**
		 * 描画対象のインデックス値を取得。
		 */
		uint32_t GetNextImage(FencePtr pfence, SemaphorePtr psemaphore, const uint64_t timeout) {

			return std::apply([&pfence, &psemaphore, timeout](VkSwapchainKHR swapchain, DevicePtr device){
				uint32_t result;
				Check(vkAcquireNextImageKHR(**device, **swapchain, timeout, **psemaphore, **pfence, &result));
				return result;
			}, Base::args);

		}
	};
	using Swapchain = _Swapchain<raii<typename identity<std::function<void(VkSwapchainKHR, DevicePtr)>>::type,VkSwapchainKHR, DevicePtr>>;
	using SwapchainPtr = shared_pointer<Swapchain>;

	class DeviceMemory {
		DevicePtr device;
		VkDeviceMemory handle;

	public:

		DeviceMemory(DevicePtr device, const uint32_t typeIndex, const VkDeviceSize size):device(device) {
			//メモリをどのようなサイズで割り当てるかといった情報
			VkMemoryAllocateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			info.pNext = nullptr;
			info.allocationSize = size;
			/*どのメモリ位置でデータ(リソース)を割り当てるかの設定
			このインデックス値は、vkGetPhysicalDeviceMemoryProperties を使用し求めることができる 
			VkPhysicalDeviceMemoryPropertiesのmemoryTypesはpropertyFlagsでメモリの種別についての情報がセットされている。
			*/
			info.memoryTypeIndex = typeIndex;

			//管理用のハンドル
			handle = VK_NULL_HANDLE;
			auto err = vkAllocateMemory(**device, &info, VK_NULL_HANDLE, &handle);
			Check(err);
		}

		~DeviceMemory() {
			vkFreeMemory(**device, handle, VK_NULL_HANDLE);
		}

		template<VkDeviceSize offset = 0>
		void bind(ImagePtr image) {
			auto err = vkBindImageMemory(**device, **image, handle, offset);
			Check(err);
		}

		template<class DataType, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags>
		void map(DataType* data) {
			auto err = vkMapMemory(**device, handle, offset, size, flags, data);
			Check(err);
		}

		void unmap() {
			vkUnmapMemory(**device, handle);
		}
	};
	using DeviceMemoryPtr = shared_pointer<DeviceMemory>;

	/**
	 * @brief コマンドバッファへの書き込みを行う際のエントリーポイント
	 * 
	 */
	class CommandRecord  {
		CommandBufferPtr m_pcmd;
		public:
			CommandRecord(CommandBufferPtr cmd, const VkCommandBufferUsageFlags flags):m_pcmd(cmd){
				/*コマンドバッファの記録を開始*/
				VkCommandBufferBeginInfo beginInfo = {};
				beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
				beginInfo.flags = flags;// VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
				/*コピー操作の実行が終了するまで、描画コマンドバッファを使用する。*/
				Check(vkBeginCommandBuffer(**m_pcmd, &beginInfo));
			}
			~CommandRecord(){
				Check(vkEndCommandBuffer(**m_pcmd));
			}
			void BindPipeline(PipelinePtr ppipeline, VkPipelineBindPoint bindpoint){
				vkCmdBindPipeline(**m_pcmd, bindpoint, **ppipeline);
			}
		private:
			static void* operator new(size_t size);
			static void operator delete(void *ptr);
	};

	/**
	 * @brief レンダーパス開始のエントリーポイント
	 * 
	 */
	class RenderRecord {
		CommandBufferPtr m_pcmd;
		public:
			RenderRecord(CommandBufferPtr cmd, RenderpassPtr renderpass, FramebufferPtr framebuffer,
				VkRect2D area, std::vector<VkClearValue> values, VkSubpassContents contents):m_pcmd(cmd){
				VkRenderPassBeginInfo info{};
				info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				info.pNext = VK_NULL_HANDLE;
				info.renderPass = **renderpass;
				info.framebuffer = **framebuffer;
				info.renderArea = area;
				info.clearValueCount = static_cast<uint32_t>(values.size());
				info.pClearValues = values.data();
				vkCmdBeginRenderPass(**m_pcmd, &info, contents);
			}
			~RenderRecord(){
				vkCmdEndRenderPass(**m_pcmd);
			}
			static RenderRecord CreateRenderRecord_Iniline(CommandBufferPtr cmd, RenderpassPtr renderpass, FramebufferPtr framebuffer,
				VkRect2D area, std::vector<VkClearValue> values){
					return RenderRecord(cmd, renderpass, framebuffer, area, values, VK_SUBPASS_CONTENTS_INLINE);
			}
			static RenderRecord CreateRenderRecord_SeccondCmdBuffer(CommandBufferPtr cmd, RenderpassPtr renderpass, FramebufferPtr framebuffer,
				VkRect2D area, std::vector<VkClearValue> values){
					return RenderRecord(cmd, renderpass, framebuffer, area, values, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
			}
		private:
			static void* operator new(size_t size);
			static void operator delete(void *ptr);
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
				return std::make_shared<Instance>([](VkInstance ptr) {
					std::cout << STR(vkDestroyInstance) << std::endl;
					vkDestroyInstance(ptr, VK_NULL_HANDLE);
					},
					instance);
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
			return std::make_shared<DebugMessanger>(
				[](VkDebugUtilsMessengerEXT ptr, InstancePtr ins ) {
					std::cout << STR(vkDestroyDebugUtilsMessengerEXT) << std::endl;
					DestroyDebugUtilsMessengerEXT(**ins, ptr, VK_NULL_HANDLE);
				}, 
				debugMsg, 
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

			return std::make_shared<Surface>( 
				[](VkSurfaceKHR ptr, InstancePtr ins) {
					std::cout << STR(vkDestroySurfaceKHR) << std::endl;
					vkDestroySurfaceKHR(**ins, ptr, VK_NULL_HANDLE);
				},
				surface,
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
			DeviceBuilder addQueueInfo(Priorities& prior,const uint32_t index) {
				auto item = VkDeviceQueueCreateInfo{};
				item.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				item.pNext = NULL;
				item.flags = 0;
				item.queueFamilyIndex = index;
				item.queueCount = static_cast<uint32_t>(prior.size());
				if (!prior.empty())
					item.pQueuePriorities = prior.data();
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

				return std::make_shared<Device>(
				[](VkDevice ptr) {
					std::cout << STR(vkDestroyDevice) << std::endl;
					vkDestroyDevice(ptr,VK_NULL_HANDLE);
				},
				logicalDevice);
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
			info = {};
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
			 IMMEDIATE_KHR : 垂直同期なしで即時に切り替えます。
			MAILBOX_KHR : 垂直同期ありで切り替えます。待ちキューが⼀杯の時、画像は置き換えとなります。
			FIFO_KHR : 垂直同期ありで切り替えます。待ちキューの最後に画像が追加となります。
			FIFO_RELAXED_KHR : 基本は垂直同期ありで切り替えですが、画像が間に合わない場合は要求が来た時点で即時切り替えになります。
			HARED_DEMAND_REFRESH_KHR:  共有表⽰可能イメージを使⽤して更新要求を出すことが出来ます。プレゼンテーションエンジンは任意の時点で処理を⾏い、ティアリングを起こすことがあります。
			SHARED_CONTINUOUS_REFRESH_KHR : 共有表⽰可能イメージを使⽤しプレゼンテーションエンジンが定期的に画⾯を更新することを指定します。但しティアリングを起こすことがあります。

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

			if (info.imageSharingMode == VK_SHARING_MODE_CONCURRENT)	{
				assert(info.pQueueFamilyIndices != nullptr);
				assert(info.queueFamilyIndexCount > 1);
			}

			VkSwapchainKHR swapchain = VK_NULL_HANDLE;
			auto err = vkCreateSwapchainKHR(**logicalDevice, &info, VK_NULL_HANDLE, &swapchain);
			Check(err);

			return std::make_shared<Swapchain>(
				[](VkSwapchainKHR ptr,DevicePtr device) {
					std::cout << STR(vkDestroySwapchainKHR) << std::endl;
					vkDestroySwapchainKHR(**device,ptr,VK_NULL_HANDLE);
				},
				swapchain,
				logicalDevice);
		}
	};//SwapchainBuilder

	/**
	 * @brief
	 *
	 */
	class ImageBuilder {
		VkImageCreateInfo info;
		DevicePtr logicalDevice;
	public:
		ImageBuilder(DevicePtr device): logicalDevice(device) {
			info = VkImageCreateInfo{};
			info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			info.pNext = nullptr;
			info.flags = 0;
		}
		static ImageBuilder get(DevicePtr device) {
			return ImageBuilder(device);
		}
		ImageBuilder flags(VkImageCreateFlags flag) {
			info.flags = flag;
			return *this;
		}
		ImageBuilder imageType(VkImageType type) {
			info.imageType = type;
			return *this;
		}
		ImageBuilder format(VkFormat format) {
			info.format = format;
			return *this;
		}
		ImageBuilder extent(VkExtent3D extent) {
			info.extent = extent;
			return *this;
		}
		ImageBuilder mipLevels(const uint32_t& levels) {
			info.mipLevels = levels;
			return *this;
		}
		ImageBuilder arrayLayers(const uint32_t& layers) {
			info.arrayLayers = layers;
			return *this;
		}
		ImageBuilder samples(VkSampleCountFlagBits bits) {
			info.samples = bits;
			return *this;
		}
		ImageBuilder tiling(VkImageTiling tiling) {
			info.tiling = tiling;
			return *this;
		}
		ImageBuilder usage(const VkImageUsageFlags flags) {
			info.usage = flags;
			return *this;
		}
		ImageBuilder sharingMode(const VkSharingMode mode) {
			info.sharingMode = mode;
			return *this;
		}
		ImageBuilder queueFamilyIndices(const cppvk::Indexs& familyInices) {
			info.queueFamilyIndexCount = static_cast<uint32_t>(familyInices.size());
			info.pQueueFamilyIndices = familyInices.data();
			return *this;
		}
		ImageBuilder initialLayout(const VkImageLayout layout) {
			info.initialLayout = layout;
			return *this;
		}
		ImagePtr build() {
			VkImage image = VK_NULL_HANDLE;
			auto err = vkCreateImage(**logicalDevice, &info, VK_NULL_HANDLE, &image);
			Check(err);

			return std::make_shared<Image>(
				[](VkImage ptr, DevicePtr device) {
					std::cout << STR(vkDestroyImage) << std::endl;
					vkDestroyImage(**device, ptr, VK_NULL_HANDLE);
				},
				image,
				logicalDevice);

		}

	};

	/**
	 * @brief 
	 * 
	 */
	class RenderpassBuilder{
		VkRenderPassCreateInfo info = {};
		DevicePtr logicalDevice;
		std::vector<VkAttachmentDescription> attachments;
		std::vector<VkSubpassDescription> descriptions;
		std::vector<VkSubpassDependency> dependencys;

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
		RenderpassBuilder addAttachments(const VkAttachmentDescription& attachment){
			attachments.push_back(attachment);
			return *this;
		}

		RenderpassBuilder addAttachments(VkAttachmentDescription&& attachment){
			attachments.push_back(attachment);
			return *this;
		}

		/**
		 * @brief 
		 * 
		 * @param 
		 * @return RenderpassBuilder 
		 */
		RenderpassBuilder addSubpassDescription(const VkSubpassDescription& description){
			descriptions.push_back(description);
			return *this;
		}

		RenderpassBuilder addSubpassDescription(VkSubpassDescription&& description){
			descriptions.push_back(description);
			return *this;
		}

		/**
		 * @brief
		 * 
		 * @param 
		 * @return RenderpassBuilder 
		 */
		RenderpassBuilder addSubpassDependency(const VkSubpassDependency& dependency){
			dependencys.push_back(dependency);
			return *this;
		}

		RenderpassBuilder addSubpassDependency(VkSubpassDependency&& dependency){
			dependencys.push_back(dependency);
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

			info.subpassCount = static_cast<uint32_t>(descriptions.size());
			if (!descriptions.empty())
				info.pSubpasses = descriptions.data();

			info.dependencyCount = static_cast<uint32_t>(dependencys.size());
			if (!dependencys.empty())
				info.pDependencies = dependencys.data();

			VkRenderPass renderPass = VK_NULL_HANDLE;
			auto err = vkCreateRenderPass(**logicalDevice,&info,VK_NULL_HANDLE,&renderPass);
			Check(err);

			return  std::make_shared<Renderpass>(
				[](VkRenderPass ptr, DevicePtr device) {
					std::cout << STR(vkDestroyRenderPass) << std::endl;
					vkDestroyRenderPass(**device, ptr, VK_NULL_HANDLE);
				},
				renderPass,
				logicalDevice);

		}

	};//RenderpassBuilder

	/**
	 * @brief
	 *
	 */
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

		/*
		TRANSIENT_BIT : 短命なコマンドバッファ⽤のプール作成時に使⽤します。
		RESET_COMMAND_BUFFER_BIT : 個別にリセット可能なコマンド⽤プール作成時に使⽤します。
		PROTECTED_BIT : 保護されたコマンドバッファ⽤で使⽤します（保護メモリ機能が有効の場合）。
		*/
		CommandPoolBuilder flags(const VkCommandPoolCreateFlags flag) {
			info.flags = flag;
			return *this;
		}

		CommandPoolBuilder queueFamilyIndices(const uint32_t& value){
			info.queueFamilyIndex = value;
			return *this;
		}

		CommandPoolPtr build(){
			VkCommandPool cmdPool = VK_NULL_HANDLE;
			auto err = vkCreateCommandPool(**logicalDevice,&info,VK_NULL_HANDLE,&cmdPool);
			Check(err);

			return std::make_shared<CommandPool>(
				[](VkCommandPool ptr, DevicePtr device) {
					std::cout << STR(vkDestroyCommandPool) << std::endl;
					vkDestroyCommandPool(**device, ptr, VK_NULL_HANDLE);
				},
				cmdPool,
				logicalDevice);
		}
	};//RenderpassBuilder

	class CommandBufferBuilder {
		VkCommandBufferAllocateInfo info = {};
		DevicePtr logicalDevice;
	public:
		CommandBufferBuilder(DevicePtr pointer):logicalDevice(pointer){
			info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			info.pNext = VK_NULL_HANDLE;
		}

		static CommandBufferBuilder get(DevicePtr pointer){
			return CommandBufferBuilder(pointer);
		}

		CommandBufferBuilder commandPool(CommandPoolPtr pool){
			info.commandPool = **pool;
			return *this;
		}

		CommandBufferBuilder level(VkCommandBufferLevel l) {
			info.level = l;
			return *this;
		}

		std::vector<CommandBufferPtr> build(const uint32_t count) {

			info.commandBufferCount = count;

			std::vector<VkCommandBuffer> cmdBuffer(count);
			auto err = vkAllocateCommandBuffers(**logicalDevice,&info,cmdBuffer.data());
			Check(err);

			//convert to buffer pointer
			std::vector<CommandBufferPtr> cmdList(count);
			for(auto i = 0; i < cmdBuffer.size(); ++i) {
				cmdList[i] = std::make_shared<CommandBuffer>(nullptr, cmdBuffer[i], logicalDevice);
			}

			return cmdList;
		}

	};

	/**
	 * @brief
	 *
	 */
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

			return std::make_shared<ImageView>(
				[](VkImageView ptr, DevicePtr device) {
					std::cout << STR(vkDestroyImageView) << std::endl;
					vkDestroyImageView(**device, ptr, VK_NULL_HANDLE);
				},
				view,
				logicalDevice);
		}

	};//ImageViewBuilder

	/**
	 * @brief
	 *
	 */
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

			return std::make_shared<ShaderModule>(
				[](VkShaderModule ptr, DevicePtr device) {
					std::cout << STR(vkDestroyShaderModule) << std::endl;
					vkDestroyShaderModule(**device, ptr, VK_NULL_HANDLE);
				},
				module,
				logicalDevice);
		}
		
	};//ShaderModuleBuilder

	/**
	 * @brief
	 *
	 */
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
			_add<VkDescriptorPoolSize>(poolSizeList,createFunc);
			return *this;
		}

		DescriptorPoolPtr build() {
			info.poolSizeCount = static_cast<uint32_t>(poolSizeList.size());
			info.pPoolSizes = poolSizeList.data();

			VkDescriptorPool pool = VK_NULL_HANDLE;
			auto err = vkCreateDescriptorPool(**logicalDevice, &info, VK_NULL_HANDLE, &pool);
			Check(err);

			return std::make_shared<DescriptorPool>(
				[](VkDescriptorPool ptr, DevicePtr device) {
					std::cout << STR(vkDestroyDescriptorPool) << std::endl;
					vkDestroyDescriptorPool(**device, ptr, VK_NULL_HANDLE);
				},
				pool,
				logicalDevice);
		}

	};//DescriptorPoolBuilder

	/**
	 * @brief
	 *
	 */
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
			info.setLayoutCount = static_cast<uint32_t>(layoutList.size());
			if(!layoutList.empty())info.pSetLayouts = layoutList.data();

			info.pushConstantRangeCount = static_cast<uint32_t>(constantRangeList.size());
			if(!constantRangeList.empty())info.pPushConstantRanges = constantRangeList.data();

			VkPipelineLayout layout = VK_NULL_HANDLE;
			auto err = vkCreatePipelineLayout(**logicalDevice, &info, VK_NULL_HANDLE, &layout);
			Check(err);

			return std::make_shared<PipelineLayout>(
				[](VkPipelineLayout ptr, DevicePtr device) {
					std::cout << STR(vkDestroyPipelineLayout) << std::endl;
					vkDestroyPipelineLayout(**device, ptr, VK_NULL_HANDLE);
				},
				layout,
				logicalDevice);
		}

		PipelineLayoutBuilder addPushConstantRange(const std::function<void(VkPushConstantRange&)> createFunc){
			_add<VkPushConstantRange>(constantRangeList, createFunc);
			return *this;
		}

		PipelineLayoutBuilder addLayout(const std::function<void(VkDescriptorSetLayout&)> createFunc){
			_add<VkDescriptorSetLayout>(layoutList, createFunc);
			return *this;
		}

	};//PiplineLayoutBuilder

	/**
	 * @brief
	 *
	 */
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
			_add<VkVertexInputBindingDescription>(vertexBindingDescriptionList, createFunc);
			return *this;
		}

		GraphicsPipelineBuilder addVertexAttributeDescription(const std::function<void(VkVertexInputAttributeDescription&)>& createFunc) {
			_add<VkVertexInputAttributeDescription>(vertexAttributeDescriptionList, createFunc);
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
			_add<VkViewport>(viewportList,createFunc);
			return *this;
		}

		GraphicsPipelineBuilder addScissors(const std::function<void(VkRect2D&)> createFunc){
			_add<VkRect2D>(scissorList,createFunc);
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
			_add<VkPipelineColorBlendAttachmentState>(attachmentList,createFunc);
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
				info.stageCount = static_cast<uint32_t>(stageList.size());
				info.pStages = stageList.data();
			}
			if(!vertexAttributeDescriptionList.empty()){
				vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttributeDescriptionList.size());
				vertexInputState.pVertexAttributeDescriptions = vertexAttributeDescriptionList.data();
			}
			if(!vertexBindingDescriptionList.empty()){
				vertexInputState.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexBindingDescriptionList.size());
				vertexInputState.pVertexBindingDescriptions = vertexBindingDescriptionList.data();
			}
			if (!viewportList.empty()) {
				viewportState.viewportCount = static_cast<uint32_t>(viewportList.size());
				viewportState.pViewports = viewportList.data();
			}
			if (!scissorList.empty()) {
				viewportState.scissorCount = static_cast<uint32_t>(scissorList.size());
				viewportState.pScissors = scissorList.data();
			}
			if (!attachmentList.empty()) {
				colorBlendState.attachmentCount = static_cast<uint32_t>(attachmentList.size());
				colorBlendState.pAttachments = attachmentList.data();
			}

			if (!dynamicStateList.empty()) { //Optional
				dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStateList.size());
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

			return std::make_shared<Pipeline>(
				[](VkPipeline ptr, DevicePtr device) {
					std::cout << STR(vkDestroyPipeline) << std::endl;
					vkDestroyPipeline(**device, ptr, VK_NULL_HANDLE);
				},
				pipeline,
				logicalDevice);
		}

	};//GraphicsPipelineBuilder

	/**
	 * @brief
	 *
	 */
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
			info.attachmentCount = static_cast<uint32_t>(attachmentList.size());
			if(!attachmentList.empty())info.pAttachments = attachmentList.data();

			VkFramebuffer buffer = VK_NULL_HANDLE;
			auto err = vkCreateFramebuffer(**logicalDevice, &info, VK_NULL_HANDLE, &buffer);
			Check(err);

			return std::make_shared<Framebuffer>(
				[](VkFramebuffer ptr, DevicePtr device) {
					std::cout << STR(vkDestroyFramebuffer) << std::endl;
					vkDestroyFramebuffer(**device, ptr, VK_NULL_HANDLE);
				},
				buffer,
				logicalDevice);
		}

	};//FrameBufferBuilder

	/**
	 * @brief CPU-GPUでの同期を行う。
	 * GPUでシグナル状態を更新、ホスト側で待機する。
	 */
	class FenceBuilder {
		VkFenceCreateInfo info;
		DevicePtr logicalDevice;
	public:
		FenceBuilder(DevicePtr pointer):logicalDevice(pointer){
			info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			info.pNext = VK_NULL_HANDLE;
		}

		static FenceBuilder get(DevicePtr pointer){
			return FenceBuilder(pointer);
		}

		FenceBuilder flag(VkFenceCreateFlags flags){
			info.flags = flags;
			return *this;
		}

		FenceBuilder next(VkExportFenceCreateInfo& einfo) {
			info.pNext = &einfo;
			return *this;
		}

		FenceBuilder next(VkExportFenceWin32HandleInfoKHR& einfo) {
			info.pNext = &einfo;
			return *this;
		}

		FencePtr build(){
			VkFence fence;
			auto err = vkCreateFence(**logicalDevice, &info, VK_NULL_HANDLE, &fence);
			Check(err);

			return std::make_shared<Fence>(
				[](VkFence ptr, DevicePtr device){
					std::cout << STR(vkDestroyFence) << std::endl;
					vkDestroyFence(**device, ptr, VK_NULL_HANDLE);
				},
				fence,
				logicalDevice);
		}

	};

	/**
	 * @brief CPU-GPUでの同期を行う。
	 * CPU/GPUどちらともシグナル状態を更新、デバイス側で待機する。
	 */
	class EventBuilder {
		VkEventCreateInfo info;
		DevicePtr logicalDevice;
	public:
		EventBuilder(DevicePtr pointer):logicalDevice(pointer) {
			info.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;
			info.pNext = VK_NULL_HANDLE;
			info.flags = 0;
		}

		static EventBuilder get(DevicePtr pointer) {
			return EventBuilder(pointer);
		}

		EventPtr build() {

			VkEvent event;
			auto err = vkCreateEvent(**logicalDevice, &info, VK_NULL_HANDLE, &event);
			Check(err);

			return std::make_shared<Event>(
				[](VkEvent ptr, DevicePtr device){
					std::cout << STR(vkDestroyEvent) << std::endl;
					vkDestroyEvent(**device, ptr, VK_NULL_HANDLE);
				},
				event,
				logicalDevice
			);
		}

	};

	/**
	 * @brief コマンドキュー間の同期で使用。
	 * GPUでシグナル状態を更新。
	 */
	class SemaphoreBuilder {
		VkSemaphoreCreateInfo info;
		DevicePtr logicalDevice;
	public:
		SemaphoreBuilder(DevicePtr pointer):logicalDevice(pointer){
			info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			info.pNext = VK_NULL_HANDLE;
			info.flags = 0;
		}

		static SemaphoreBuilder get(DevicePtr pointer) {
			return SemaphoreBuilder(pointer);
		}

		SemaphoreBuilder next(VkExportSemaphoreCreateInfo& einfo) {
			info.pNext = &einfo;
			return *this;
		}

		SemaphoreBuilder next(VkExportSemaphoreWin32HandleInfoKHR& einfo) {
			info.pNext = &einfo;
			return *this;
		}

		SemaphorePtr build() {
			return nullptr;
		}

	};

}

#pragma warning(pop)