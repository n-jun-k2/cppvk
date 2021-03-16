#pragma once

namespace cppvk {

  /// <summary>
  /// Allocate memory using method chain
  /// </summary>
  /// <typeparam name="R">Object type to return after allocation processing</typeparam>
  /// <typeparam name="T">info type</typeparam>
  template<typename R, typename T, R&(*_allocate)(T*)>
  class Allocater {
  protected:
    T m_info;

  public:
    Allocater() : m_info({}){}
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
      return _allocate(m_info);
    }
  };
}