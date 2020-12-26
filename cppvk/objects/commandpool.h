#pragma once

#include "../vk.h"
#include "../context.h"
#include "object.h"

#include "../allocater/commandbuffer.h"

#include <vector>

namespace cppvk {

  class CommandPool : public Object {
    private:
      VkCommandPool cmdpool;
      std::vector<VkCommandBuffer> cmdbuffferList;

      void destroyBuffer() {
        if (cmdbuffferList.empty())
          return;

        std::cout << "vkFreeCommandBuffer" << std::endl;
        vkFreeCommandBuffers(context->device, cmdpool, static_cast<uint32_t>(cmdbuffferList.size()), cmdbuffferList.data());
      }

    public:
      using Ptr = std::shared_ptr<CommandPool>;
      using AllocateInfo = cppvk::CommandBufferAllocate<CommandPool>;

      CommandPool(cppvk::Context::Ptr ptr, VkCommandPool pool) : cmdpool(pool), Object(ptr, nullptr), cmdbuffferList({}) {

      }

      ~CommandPool() {

        destroyBuffer();

        std::cout << "vkDestroyCommandPool" << std::endl;
        vkDestroyCommandPool(context->device, cmdpool, VK_NULL_HANDLE);
      }

      CommandPool(const CommandPool&) = default;
      CommandPool& operator=(const CommandPool&) = default;
      CommandPool(CommandPool&&) = default;
      CommandPool& operator=(CommandPool&&) = default;

      AllocateInfo allocateinfo() {

        typename AllocateInfo::AllocateFunc f = [=](VkCommandBufferAllocateInfo& info) ->cppvk::CommandPool& {

          destroyBuffer();

          this->cmdbuffferList.resize(info.commandBufferCount);
          info.commandPool = this->cmdpool;
          auto err = vkAllocateCommandBuffers(this->context->device, &info, this->cmdbuffferList.data());
          checkVk(err);
          return *this;
        };

        return AllocateInfo(f);
      }

  };

}