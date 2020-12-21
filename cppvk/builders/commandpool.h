#pragma once

#include "../vk.h"
#include "../context.h"
#include "builder.h"

#include "../objects/commandpool.h"

namespace cppvk {

  /// <summary>
  /// 
  /// </summary>
  class CommandPoolBuilder: public cppvk::Builder {

    private:
      VkCommandPoolCreateInfo info;


      virtual cppvk::CommandPool* createimpl(const VkAllocationCallbacks* arg) override {

      }

    public:

      cppvk::CommandPool::Ptr create(const VkAllocationCallbacks* callbacks = VK_NULL_HANDLE) {

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

  };

}