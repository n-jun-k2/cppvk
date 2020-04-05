# cppvk
vulkanとGLFW3の練習。

## VkInstance flow to create...
Process to create vkinstance.
```
auto instance = cppvk::InstanceBuilder::get()
			.enabledExtensionNames(extensions)
			.enabledLayerNames(validationLayers)
			.build();
```

## VkSurfaceKHR flow to create...
Processing to create vulkan surface in windows environment.
```
auto surface = cppvk::WinSurfaceBuilder::get(instance)
		    .hwnd(wPtr)
		    .build();
```

## VkPhysicalDeviceProperties flow to get...

```
auto device = cppvk::helper::chooseGpu(instance,
    [](VkPhysicalDeviceProperties props, VkPhysicalDeviceFeatures features) {
		return //Describe the physical device selection conditions.
	});
```

## 