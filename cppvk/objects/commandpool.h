#pragma once

#include "../vk.h"
#include "../context.h"
#include "object.h"

namespace cppvk {

  class CommandPool : public Object {
    public:
      using Object::Object;
      using Ptr = std::shared_ptr<CommandPool>;
  };

}