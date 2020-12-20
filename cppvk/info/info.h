#pragma once

namespace cppvk {

  /// <summary>
  /// Provide an implicit cast
  /// </summary>
  /// <typeparam name="T"></typeparam>
  template <class T>
  class Info {

    protected:
      T info;

    public:

      Info()                        = default;
      virtual ~Info()               = default;
      Info(const Info&)             = default;
      Info& operator=(const Info&)  = default;
      Info(Info&&)                  = default;
      Info& operator=(Info&&)       = default;

      operator T() const & {
        return info;
      }

      operator T() const && {
        return std::move(info);
      }

  };

}