#pragma once

#include "../vk.h"
#include "../context.h"
#include "../objects/object.h"

#include <memory>

namespace cppvk {


  /// <summary>
  /// 
  /// </summary>
  class  Builder {

  private:

    static void* operator new(size_t ) {}
    static void operator delete(void* ) {}

    /// <summary>
    /// 
    /// </summary>
    /// <param name=""></param>
    /// <returns></returns>
    virtual cppvk::Object* createimpl(const VkAllocationCallbacks*) = 0;

  protected:

    cppvk::Context::Ref context;

  public:
    Builder(const  Builder&)            = default;
    Builder& operator=(const  Builder&) = default;
    Builder( Builder&&)                 = default;
    Builder& operator=( Builder&&)      = default;
    Builder()                           = delete;
    virtual ~ Builder()                 = default;
    explicit  Builder(cppvk::Context::Ref ctx): context(ctx) {}

    /// <summary>
    /// 
    /// </summary>
    /// <param name="callbacks"></param>
    /// <returns></returns>
    cppvk::Object::Ptr create(const VkAllocationCallbacks* callbacks) {
      return cppvk::Object::Ptr(this->createimpl(callbacks));
    }

  };

}