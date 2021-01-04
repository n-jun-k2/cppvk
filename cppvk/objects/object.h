#pragma once

#include "../vk.h"
#include "../destroy.h"

#include <memory>
#include <functional>

namespace cppvk {

  /// <summary>
  /// Objects for manipulating Vulkan objects
  /// </summary>
  class Object {

  public:

    using pointer = std::shared_ptr<Object>;
    using reference = std::weak_ptr<Object>;

    explicit Object(pointer obj) : onObject(obj) { }
    Object() = delete;
    Object(const Object&) = default;
    Object& operator=(const Object&)  = default;
    Object(Object&&) = default;
    Object& operator=(Object&&) = default;
    virtual ~Object() = default;

  protected:
      pointer onObject;
  };

}