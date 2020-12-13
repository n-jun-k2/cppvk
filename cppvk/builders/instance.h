#pragma once

#include "../vk.h"
#include "builder.h"
#include "../objects/instance.h"
#include "../physicaldeviceset.h"
#include <string>
#include <cassert>
#include <stdexcept>


namespace cppvk {

  /// <summary>
  /// 
  /// </summary>
  class InstanceBuilder : public cppvk::Builder
  {

  private:
    VkInstanceCreateInfo info;
    VkApplicationInfo appInfo;

    /// <summary>
    /// 
    /// </summary>
    /// <param name="callbacks"></param>
    /// <returns></returns>
    Instance* createimpl(const VkAllocationCallbacks* callbacks = VK_NULL_HANDLE) {

      if (auto ctx = this->context.lock()) {

        checkVk(vkCreateInstance(&info, callbacks, &ctx->instance));

        return new Instance(ctx, [=](cppvk::Context& p) {
          std::cout << "vkDestroyInstance" << std::endl;
          vkDestroyInstance(p.instance, callbacks);
          });

      }

      throw std::runtime_error("Vulkan context does not exist");

    }

  public:

    InstanceBuilder(cppvk::Context::Ptr ctx) : cppvk::Builder(ctx) {
      appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
      appInfo.pNext = VK_NULL_HANDLE;
      appInfo.pApplicationName = NULL;
      appInfo.pEngineName = NULL;

      info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
      info.pNext = VK_NULL_HANDLE;
      info.flags = 0;
      info.pApplicationInfo = &appInfo;
      info.enabledLayerCount = 0;
      info.ppEnabledLayerNames = NULL;
      info.enabledExtensionCount = 0;
      info.ppEnabledExtensionNames = NULL;
    }
    ~InstanceBuilder() = default;
    InstanceBuilder()                                   = delete;
    InstanceBuilder(const InstanceBuilder&)             = default;
    InstanceBuilder& operator=(const InstanceBuilder&)  = default;
    InstanceBuilder(InstanceBuilder&&)                  = default;
    InstanceBuilder& operator=(InstanceBuilder&&)       = default;

    /// <summary>
    ///  pApplicationName is NULL or is a pointer to a null-terminated UTF-8 string containing the name of the application.
    /// </summary>
    /// <param name="name"></param>
    /// <returns>InstanceBuilder</returns>
    InstanceBuilder applicationName(const std::string& name) {
      appInfo.pApplicationName = name.c_str();
      return *this;
    }

    /// <summary>
    /// pEngineName is NULL or is a pointer to a null-terminated UTF-8 string containing the name of the engine (if any) used to create the application.
    /// </summary>
    /// <param name="name"></param>
    /// <returns></returns>
    InstanceBuilder engineName(const std::string& name) {
      appInfo.pEngineName = name.c_str();
      return *this;
    }


    /// <summary>
    /// applicationVersion is an unsigned integer variable containing the developer-supplied version number of the application.
    /// </summary>
    /// <param name="version"></param>
    /// <returns></returns>
    InstanceBuilder applicationVersion(const uint32_t version) {
      appInfo.applicationVersion = version;
      return *this;
    }


    /// <summary>
    /// engineVersion is an unsigned integer variable containing the developer-supplied version number of the engine used to create the application.
    /// </summary>
    /// <param name="version"></param>
    /// <returns></returns>
    InstanceBuilder engineVersion(const uint32_t version) {
      appInfo.engineVersion = version;
      return *this;
    }


    /// <summary>
    ///  apiVersion is the version of the Vulkan API against which the application expects to run, encoded as described in the API Version Numbers and Semantics section. If apiVersion is 0 the implementation must ignore it, otherwise if the implementation does not support the requested apiVersion, or an effective substitute for apiVersion, it must return VK_ERROR_INCOMPATIBLE_DRIVER. The patch version number specified in apiVersion is ignored when creating an instance object. Only the major and minor versions of the instance must match those requested in apiVersion.
    /// </summary>
    /// <param name="version"></param>
    /// <returns></returns>
    InstanceBuilder apiVersion(const uint32_t version) {
      appInfo.apiVersion = version;
      return *this;
    }


    /// <summary>
    /// ppEnabledLayerNames is a pointer to an array of enabledLayerCount null-terminated UTF-8 strings containing the names of layers to enable for the created instance. See the Layers section for further details.
    /// </summary>
    /// <param name="layers"></param>
    /// <returns></returns>
    InstanceBuilder enabledLayerNames(const Names& layers) {
      info.enabledLayerCount = static_cast<uint32_t>(layers.size());
      if (!layers.empty())info.ppEnabledLayerNames = layers.data();
      return *this;
    }


    /// <summary>
    /// ppEnabledExtensionNames is a pointer to an array of enabledExtensionCount null-terminated UTF-8 strings containing the names of extensions to enable.
    /// </summary>
    /// <param name="extensions"></param>
    /// <returns></returns>
    InstanceBuilder enabledExtensionNames(const Names& extensions) {
      info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
      if (!extensions.empty())info.ppEnabledExtensionNames = extensions.data();
      return *this;
    }

    /// <summary>
    /// 
    /// </summary>
    /// <param name="callbacks"></param>
    /// <returns></returns>
    Instance::Ptr create(const VkAllocationCallbacks* callbacks = VK_NULL_HANDLE) {
      return Instance::Ptr(this->createimpl(callbacks));
    }

  };//InstanceBuilder

}