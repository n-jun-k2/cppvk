#pragma once

#include "../vk.h"
#include "../context.h"
#include "object.h"
#include <memory>

namespace cppvk {

  /// <summary>
  /// 
  /// </summary>
  class Device : public Object {
  public:
    using Object::Object;
    using Ptr = std::shared_ptr<Device>;
  };

}