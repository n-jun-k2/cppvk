#pragma once

#include "../vk.h"
#include "../context.h"
#include "builder.h"

#include "../objects/commandpool.h"

#include <iostream>

namespace cppvk {

  /// <summary>
  ///
  /// </summary>
  class CommandPoolBuilder: public cppvk::Builder {

    private:
      VkCommandPoolCreateInfo info;

      /// <summary>
      /// 
      /// </summary>
      /// <param name="arg"></param>
      /// <returns></returns>
      virtual cppvk::CommandPool* createimpl(const VkAllocationCallbacks* arg) override {

        if (auto ctx = context.lock()){

          const auto err = vkCreateCommandPool(ctx->device, &info, arg, &ctx->commandpool);
          checkVk(err);

          return new cppvk::CommandPool(ctx, [=](cppvk::Context& p) {
            std::cout << "vkDestroyCommandPool" << std::endl;
            vkDestroyCommandPool(p.device, p.commandpool, arg);
            });

        }

        throw std::runtime_error("Vulkan context does not exist");
      }

    public:

      /// <summary>
      /// Create CommandPool
      /// </summary>
      /// <param name="callbacks"></param>
      /// <returns></returns>
      cppvk::CommandPool::Ptr create(const VkAllocationCallbacks* callbacks = VK_NULL_HANDLE) {
        return cppvk::CommandPool::Ptr(createimpl(callbacks));
      }

      explicit CommandPoolBuilder(cppvk::Context::Ptr ctx): cppvk::Builder(ctx) {

        info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        info.pNext = VK_NULL_HANDLE;
        info.flags = 0;

      }

      ~CommandPoolBuilder()                                     = default;
      CommandPoolBuilder()                                      = delete;
      CommandPoolBuilder(const CommandPoolBuilder&)             = default;
      CommandPoolBuilder& operator=(const CommandPoolBuilder&)  = default;
      CommandPoolBuilder(CommandPoolBuilder&&)                  = default;
      CommandPoolBuilder& operator=(CommandPoolBuilder&&)       = default;

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

}