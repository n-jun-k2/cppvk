#pragma once

#include "../vk.h"
#include "../objects/object.h"

#include <memory>

namespace cppvk {


  /// <summary>
  /// 
  /// </summary>
  class  IBuilder {

  private:

    static void* operator new(size_t ) {}
    static void operator delete(void* ) {}

    virtual cppvk::Object* createimpl(const VkAllocationCallbacks*) = 0;

  public:
    IBuilder(const  IBuilder&) = default;
    IBuilder& operator=(const  IBuilder&) = default;
    IBuilder(IBuilder&&) = default;
    IBuilder& operator=(IBuilder&&) = default;
    IBuilder() = default;
    virtual ~IBuilder() = default;

    typename cppvk::Object::pointer create(const VkAllocationCallbacks* callbacks) {
      return typename cppvk::Object::pointer(this->createimpl(callbacks));
    }

  };

}