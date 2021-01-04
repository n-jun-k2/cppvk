#pragma once

#include <functional>

namespace cppvk {

  /// <summary>
  /// Functions managed by the destroy object
  /// </summary>
  using DestoryFunc = std::function<void()>;

  /// <summary>
  /// Object that manages deletion process by function join
  /// </summary>
  class Destroy {

  public:
    explicit Destroy() : destroy([]() {}) {}
    ~Destroy() { destroy();  }

    /// <summary>
    /// Interface to add termination processing for Vulkan objects
    /// </summary>
    /// <param name="other">Delete process</param>
    void operator+=(DestoryFunc other) {
      const auto& pre = destroy;
      destroy = [=]() { other(); pre(); };
    }

  private:
    DestoryFunc destroy;
  };
}