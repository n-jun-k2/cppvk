#pragma once

#include "vk.h"
#include <variant>

namespace cppvk {

  using vkVariant = std::variant<
    VkDeviceMemory,
    VkAccelerationStructureKHR,
    VkAccelerationStructureNV,
    VkBuffer,
    VkBufferView,
    VkCommandPool,
    VkDebugReportCallbackEXT,
    VkDebugUtilsMessengerEXT,
    VkDescriptorPool,
    VkDescriptorSetLayout,
    VkDescriptorUpdateTemplate,
    VkDescriptorUpdateTemplate,
    VkDevice,
    VkDisplayKHR,
    VkDisplayModeKHR,
    VkEvent,
    VkFence,
    VkFramebuffer,
    VkImage,
    VkImageView,
    VkIndirectCommandsLayoutNV,
    VkInstance,
    VkPipeline,
    VkPipelineCache,
    VkPrivateDataSlotEXT,
    VkQueryPool,
    VkRenderPass,
    VkSampler,
    VkSamplerYcbcrConversion,
    VkSamplerYcbcrConversionKHR,
    VkSemaphore,
    VkShaderModule,
    VkSurfaceKHR,
    VkSwapchainKHR,
    VkValidationCacheEXT,
    VkAccelerationStructureKHR,
  >;

}