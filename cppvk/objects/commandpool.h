#pragma once

#include "../vk.h"
#include "object.h"
#include "logicaldevice.h"

#include "../allocator/commandbuffer.h"

#include <vector>

namespace cppvk {
  class LogicalDevice::CommandPool : public cppvk::Object {
    public:
      using Object::Object;
      using pointer = std::shared_ptr<CommandPool>;
      using reference = std::weak_ptr<CommandPool>;
      using AllocateInfo = cppvk::CommandBufferAllocate<CommandPool>;
      class CommandPoolBuilder;

      ~CommandPool() {
        if (this->onObject) {
          auto pDevice = std::get<1>(this->onObject.value());
          freeBuffer(pDevice->device);
        }
      }

      AllocateInfo getCommandBufferAllocator() {

        auto objectPointer = this->onObject;
        auto pDevice = std::get<1>(objectPointer.value());
        auto vkDevice = pDevice->device;

        freeBuffer(vkDevice);

        typename AllocateInfo::AllocateFunc f = [=](VkCommandBufferAllocateInfo& info) ->CommandPool& {
          bufferList.resize(info.commandBufferCount);
          info.commandPool = commandPool;
          checkVk(vkAllocateCommandBuffers(vkDevice, &info, bufferList.data()));
          return *this;
        };

        return AllocateInfo(f);
      }

    private:
      VkCommandPool commandPool;
      std::vector<VkCommandBuffer> bufferList;

      void freeBuffer(VkDevice device) {
        if (bufferList.empty())
          return;
        std::cout << "vkFreeCommandBuffer" << bufferList.data() << std::endl;
        vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(bufferList.size()), bufferList.data());
      }

  };

  using CommandPool = LogicalDevice::CommandPool;
}