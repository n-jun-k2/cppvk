#pragma once

#include "vk.h"
#include "type.h"

namespace cppvk {
  using DescriptorSetLayoutPool = Pool<VkDescriptorSetLayout, std::vector>;
  using DescriptorSetLayoutPoolPtr = std::shared_ptr<DescriptorSetLayoutPool>;
  using DescriptorSetLayoutPoolRef = std::weak_ptr<DescriptorSetLayoutPool>;
}