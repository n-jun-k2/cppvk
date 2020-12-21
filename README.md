# cppvk

Valkan and GLFW3 practice.

## Preparation
Create a context object for the Vulkan object. An instance of the Vulkan object and each termination process are registered in this object.

It is managed by shared_ptr and will not be deleted as long as there is at least one "Object" instance.
```cpp
auto m_ctx = cppvk::Context::make();
```
## VkInstance flow to create...
Process to create vkinstance.
```cpp
auto instance = cppvk::InstanceBuilder(m_ctx)
			.enabledExtensionNames(extensions)
			.enabledLayerNames(validationLayers)
			.build();
```

## VkSurfaceKHR flow to create...
Processing to create vulkan surface in windows environment.
```cpp
auto surface = cppvk::WinSurfaceBuilder(m_ctx)
		    .hwnd(wPtr)
		    .build();
```

## VkPhysicalDeviceProperties flow to get...

```cpp
auto device_set = cppvk::helper::chooseGpu(instance,
    [](VkPhysicalDeviceProperties props, VkPhysicalDeviceFeatures features) {
		return //Describe the physical device selection conditions.
	});
```

## VkDevice flow to create...
```cpp
#include "cppvk/builders/device.h"
#include "cppvk/objects/device.h"
#include "cppvk/info/devicequeue.h"
...
auto m_device = cppvk::DeviceBuilder(m_ctx, physical_device)
      .addQueueInfo(
        cppvk::DeviceQueueInfo()
        .queuePriorities({1.0f})
        .familyIndex(graphics_queue_index))
      .extensions(dev_extension)
      .layerNames(validation_layers)
      .features(physical_device.getFeatures())
      .create();
```