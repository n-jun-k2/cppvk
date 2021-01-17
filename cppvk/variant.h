#pragma once

#include <memory>
#include <variant>
#include <optional>


namespace cppvk {

  template<class... Args>
  using variant = std::optional<std::variant<Args...>>;

  class Instance;
  class LogicalDevice;

  using VkBaseVariant = variant<std::shared_ptr<Instance>, std::shared_ptr<LogicalDevice>>;

}