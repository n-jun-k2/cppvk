#pragma once
#include <initializer_list>
#include <memory>

namespace cppvk {

  class Nondynamicallocation {
    protected:
      Nondynamicallocation() = default;
      ~Nondynamicallocation() = default;
      static void* operator new(size_t ) {}
      static void operator delete(void* ) {}
  };

  class Noncopyable {
    protected:
      Noncopyable() = default;
      ~Noncopyable() = default;
      Noncopyable(const Noncopyable&) = delete;
      Noncopyable& operator=(const Noncopyable&) = delete;
  };

  template<typename Type, typename Wtype,
    template<typename E, typename Allocator = std::allocator<E>>class Container>
  union UnionWrapper {
    Container<Wtype> values;
    Container<Type> row;
    UnionWrapper() : values() {}
    UnionWrapper(std::initializer_list<Wtype> init) :
      values(std::begin(init), std::end(init)) {}
    ~UnionWrapper() {}
  };

}