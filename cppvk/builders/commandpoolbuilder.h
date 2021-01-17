#pragma once

#include "../vk.h"
#include "../objects/logicaldevice.h"
#include "../objects/commandpool.h"
#include "Ibuilder.h"

namespace cppvk {

  class CommandPool::CommandPoolBuilder : public cppvk::IBuilder{

  private:
    VkCommandPoolCreateInfo info;
    cppvk::LogicalDevice::reference object;

    virtual cppvk::CommandPool* createimpl(const VkAllocationCallbacks*arg ) override {

      if (auto pLogicalDevice = this->object.lock()) {

        auto pCommandPool = new CommandPool(pLogicalDevice);
        auto& vkDevice = pLogicalDevice->device;
        auto& vkCommandPool = pCommandPool->commandPool;

        checkVk(vkCreateCommandPool(vkDevice, &info, arg, &vkCommandPool));

        *(pLogicalDevice->destroy) += [=]() {
          std::cout << "vkDestroyCommandPool : " << vkCommandPool << std::endl;
          vkDestroyCommandPool(vkDevice, vkCommandPool, arg);
        };

        return pCommandPool;

      }
      throw std::runtime_error("Vulkan context does not exist");
    }

  public:

    CommandPoolBuilder() = delete;
    CommandPoolBuilder(const CommandPoolBuilder&) = default;
    CommandPoolBuilder& operator=(const CommandPoolBuilder&) = default;
    CommandPoolBuilder(CommandPoolBuilder&&) = default;
    CommandPoolBuilder& operator=(CommandPoolBuilder&&) = default;

    explicit CommandPoolBuilder(cppvk::LogicalDevice::reference refInstance) : object(refInstance) {

      info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
      info.pNext = VK_NULL_HANDLE;
      info.flags = 0;

    }

    CommandPool::pointer create(const VkAllocationCallbacks* callbacks = VK_NULL_HANDLE) {
      return CommandPool::pointer(this->createimpl(callbacks));
    }

    /// <summary>
    /// flags is a bitmask of VkCommandPoolCreateFlagBits indicating usage behavior for the pool and command buffers allocated from it.
    /// </summary>
    /// <param name="flag"></param>
    /// <returns></returns>
    CommandPoolBuilder& flags(const VkCommandPoolCreateFlags flag) {
      info.flags = flag;
      return *this;
    }

    /// <summary>
    /// eueFamilyIndex designates a queue family as described in section Queue Family Properties.
    ///  All command buffers allocated from this command pool must be submitted on queues from the same queue family.
    /// </summary>
    /// <param name="value"></param>
    /// <returns></returns>
    CommandPoolBuilder& queueFamilyIndices(const uint32_t& value){
      info.queueFamilyIndex = value;
      return *this;
    }

  };

  using CommandPoolBuilder = CommandPool::CommandPoolBuilder;

}