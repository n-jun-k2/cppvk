#pragma once

#include "../vk.h"
#include "../context.h"

namespace cppvk {

  /// <summary>
  /// Objects for manipulating Vulkan objects
  /// </summary>
  class Object {

  protected:

    cppvk::Context::Ptr context;

  public:

    using Ptr = std::shared_ptr<Object>;

    Object()                          = delete;
    Object(const Object&)             = default;
    Object& operator=(const Object&)  = default;
    Object(Object&&)                  = default;
    Object& operator=(Object&&)       = default;
    virtual ~Object()                 = default;

    /// <summary>
    /// constructor
    /// </summary>
    /// <param name="ctx">vulkan context object pointer</param>
    /// <param name="destroy">Vulkan object instance deletion process</param>
    explicit Object(cppvk::Context::Ptr ctx, cppvk::Context::DestoryFunc destroy) :context(ctx){
      *ctx += destroy;
    }

  };

}