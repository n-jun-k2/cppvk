#pragma once

#include <functional>

namespace cppvk {

  /// <summary>
  /// Allocate memory using method chain
  /// </summary>
  /// <typeparam name="R">Object type to return after allocation processing</typeparam>
  /// <typeparam name="T">info type</typeparam>
  template<typename R, typename T>
  class Allocater {
  public:
    using AllocateFunc = std::function<R&(T&)>;

  private:
    AllocateFunc m_allocate;

  protected:
    T m_info;

  public:
    explicit Allocater(AllocateFunc arg) : m_info({}), m_allocate(arg) {}
    virtual ~Allocater() = default;

    Allocater(const Allocater&) = default;
    Allocater& operator=(const Allocater&) = default;
    Allocater(Allocater&&) = default;
    Allocater& operator=(Allocater&&) = default;

    /// <summary>
    /// Allocation interface
    /// </summary>
    /// <returns></returns>
    R& allocate() {
      return this->m_allocate(m_info);
    }

  };

}