#pragma once

#include "type.h"

#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>

#include <fstream>
#include <memory>

namespace cppvk {

  class GlslangTools:public Singleton<GlslangTools> {
    private:
      TBuiltInResource m_builtinResource;
      glslang::EShTargetClientVersion m_targetClientVersion;
      glslang::EShTargetLanguageVersion m_targetLanguageVersion;

    protected:
      GlslangTools() = default;
      virtual GlslangTools* createInstance() override {
        if(glslang::InitializeProcess()) {
          std::runtime_error("Failed to ShaderLang initialize the process.");
        }
        auto instance = new GlslangTools;
        instance->m_targetClientVersion = glslang::EShTargetClientVersion::EShTargetVulkan_1_2;
        instance->m_targetLanguageVersion = glslang::EShTargetLanguageVersion::EShTargetSpv_1_4;
        instance->initResources();
        return instance;
      }

    public:
      ~GlslangTools() {
        glslang::FinalizeProcess();
      }

      template<template<typename E, typename Allocator=std::allocator<E>>typename Container>
      void readFile(const std::string& filePath, Container<char>& buffer) {
        std::ifstream file(filePath, std::ios::ate | std::ios::in);
        if (!file.is_open())
          throw std::runtime_error("failed to open file!");

        auto fileSize = (size_t)file.tellg();
        buffer.resize(fileSize);
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();
      }

      template<template<typename E, typename Allocator=std::allocator<E>>typename Container>
      void glsltoSpv(const EShLanguage codeType, const Container<char>& code, std::vector<uint32_t>& bin) {
        const auto message = static_cast<EShMessages>(EShMsgSpvRules | EShMsgVulkanRules);

        glslang::SpvOptions options;
        options.generateDebugInfo = true;
        options.disableOptimizer = false;
        options.optimizeSize = false;

        glslang::TShader shaderObject(codeType);
        std::string strCode(code.begin(), code.end());
        std::vector<const char*> pShaderSrcs {
          strCode.c_str()
        };
        shaderObject.setStrings(pShaderSrcs.data(), static_cast<int>(pShaderSrcs));
        shaderObject.setEnvInput(glslang::EShSource::EShSourceGlsl, codeType, glslang::EShClient::EShClientVulkan, m_targetClientVersion);
        shaderObject.setEnvClient(glslang::EShClient::EShClientVulkan, m_targetClientVersion);
        shaderObject.setEnvTarget(glslang::EShTargetLanguage::EShTargetSpv, m_targetLanguageVersion);
        if(!shaderObject.parse(&m_builtinResource, 100, false, message)) {
          throw std::runtime_error(shaderObject.getInfoLog());
        }

        glslang::TProgram program;
        program.addShader(&shaderObject);
        if(!program.link(message)){
          throw std::runtime_error(program.getInfoLog());
        }

        glslang::GlslangToSpv(*program.getIntermediate(codeType), bin, &options);

      }

      void initResources() noexcept{
          m_builtinResource.maxLights                                 = 32;
          m_builtinResource.maxClipPlanes                             = 6;
          m_builtinResource.maxTextureUnits                           = 32;
          m_builtinResource.maxTextureCoords                          = 32;
          m_builtinResource.maxVertexAttribs                          = 64;
          m_builtinResource.maxVertexUniformComponents                = 4096;
          m_builtinResource.maxVaryingFloats                          = 64;
          m_builtinResource.maxVertexTextureImageUnits                = 32;
          m_builtinResource.maxCombinedTextureImageUnits              = 80;
          m_builtinResource.maxTextureImageUnits                      = 32;
          m_builtinResource.maxFragmentUniformComponents              = 4096;
          m_builtinResource.maxDrawBuffers                            = 32;
          m_builtinResource.maxVertexUniformVectors                   = 128;
          m_builtinResource.maxVaryingVectors                         = 8;
          m_builtinResource.maxFragmentUniformVectors                 = 16;
          m_builtinResource.maxVertexOutputVectors                    = 16;
          m_builtinResource.maxFragmentInputVectors                   = 15;
          m_builtinResource.minProgramTexelOffset                     = -8;
          m_builtinResource.maxProgramTexelOffset                     = 7;
          m_builtinResource.maxClipDistances                          = 8;
          m_builtinResource.maxComputeWorkGroupCountX                 = 65535;
          m_builtinResource.maxComputeWorkGroupCountY                 = 65535;
          m_builtinResource.maxComputeWorkGroupCountZ                 = 65535;
          m_builtinResource.maxComputeWorkGroupSizeX                  = 1024;
          m_builtinResource.maxComputeWorkGroupSizeY                  = 1024;
          m_builtinResource.maxComputeWorkGroupSizeZ                  = 64;
          m_builtinResource.maxComputeUniformComponents               = 1024;
          m_builtinResource.maxComputeTextureImageUnits               = 16;
          m_builtinResource.maxComputeImageUniforms                   = 8;
          m_builtinResource.maxComputeAtomicCounters                  = 8;
          m_builtinResource.maxComputeAtomicCounterBuffers            = 1;
          m_builtinResource.maxVaryingComponents                      = 60;
          m_builtinResource.maxVertexOutputComponents                 = 64;
          m_builtinResource.maxGeometryInputComponents                = 64;
          m_builtinResource.maxGeometryOutputComponents               = 128;
          m_builtinResource.maxFragmentInputComponents                = 128;
          m_builtinResource.maxImageUnits                             = 8;
          m_builtinResource.maxCombinedImageUnitsAndFragmentOutputs   = 8;
          m_builtinResource.maxCombinedShaderOutputResources          = 8;
          m_builtinResource.maxImageSamples                           = 0;
          m_builtinResource.maxVertexImageUniforms                    = 0;
          m_builtinResource.maxTessControlImageUniforms               = 0;
          m_builtinResource.maxTessEvaluationImageUniforms            = 0;
          m_builtinResource.maxGeometryImageUniforms                  = 0;
          m_builtinResource.maxFragmentImageUniforms                  = 8;
          m_builtinResource.maxCombinedImageUniforms                  = 8;
          m_builtinResource.maxGeometryTextureImageUnits              = 16;
          m_builtinResource.maxGeometryOutputVertices                 = 256;
          m_builtinResource.maxGeometryTotalOutputComponents          = 1024;
          m_builtinResource.maxGeometryUniformComponents              = 1024;
          m_builtinResource.maxGeometryVaryingComponents              = 64;
          m_builtinResource.maxTessControlInputComponents             = 128;
          m_builtinResource.maxTessControlOutputComponents            = 128;
          m_builtinResource.maxTessControlTextureImageUnits           = 16;
          m_builtinResource.maxTessControlUniformComponents           = 1024;
          m_builtinResource.maxTessControlTotalOutputComponents       = 4096;
          m_builtinResource.maxTessEvaluationInputComponents          = 128;
          m_builtinResource.maxTessEvaluationOutputComponents         = 128;
          m_builtinResource.maxTessEvaluationTextureImageUnits        = 16;
          m_builtinResource.maxTessEvaluationUniformComponents        = 1024;
          m_builtinResource.maxTessPatchComponents                    = 120;
          m_builtinResource.maxPatchVertices                          = 32;
          m_builtinResource.maxTessGenLevel                           = 64;
          m_builtinResource.maxViewports                              = 16;
          m_builtinResource.maxVertexAtomicCounters                   = 0;
          m_builtinResource.maxTessControlAtomicCounters              = 0;
          m_builtinResource.maxTessEvaluationAtomicCounters           = 0;
          m_builtinResource.maxGeometryAtomicCounters                 = 0;
          m_builtinResource.maxFragmentAtomicCounters                 = 8;
          m_builtinResource.maxCombinedAtomicCounters                 = 8;
          m_builtinResource.maxAtomicCounterBindings                  = 1;
          m_builtinResource.maxVertexAtomicCounterBuffers             = 0;
          m_builtinResource.maxTessControlAtomicCounterBuffers        = 0;
          m_builtinResource.maxTessEvaluationAtomicCounterBuffers     = 0;
          m_builtinResource.maxGeometryAtomicCounterBuffers           = 0;
          m_builtinResource.maxFragmentAtomicCounterBuffers           = 1;
          m_builtinResource.maxCombinedAtomicCounterBuffers           = 1;
          m_builtinResource.maxAtomicCounterBufferSize                = 16384;
          m_builtinResource.maxTransformFeedbackBuffers               = 4;
          m_builtinResource.maxTransformFeedbackInterleavedComponents = 64;
          m_builtinResource.maxCullDistances                          = 8;
          m_builtinResource.maxCombinedClipAndCullDistances           = 8;
          m_builtinResource.maxSamples                                = 4;
          m_builtinResource.maxMeshOutputVerticesNV                   = 256;
          m_builtinResource.maxMeshOutputPrimitivesNV                 = 512;
          m_builtinResource.maxMeshWorkGroupSizeX_NV                  = 32;
          m_builtinResource.maxMeshWorkGroupSizeY_NV                  = 1;
          m_builtinResource.maxMeshWorkGroupSizeZ_NV                  = 1;
          m_builtinResource.maxTaskWorkGroupSizeX_NV                  = 32;
          m_builtinResource.maxTaskWorkGroupSizeY_NV                  = 1;
          m_builtinResource.maxTaskWorkGroupSizeZ_NV                  = 1;
          m_builtinResource.maxMeshViewCountNV                        = 4;

          m_builtinResource.limits.nonInductiveForLoops                 = 1;
          m_builtinResource.limits.whileLoops                           = 1;
          m_builtinResource.limits.doWhileLoops                         = 1;
          m_builtinResource.limits.generalUniformIndexing               = 1;
          m_builtinResource.limits.generalAttributeMatrixVectorIndexing = 1;
          m_builtinResource.limits.generalVaryingIndexing               = 1;
          m_builtinResource.limits.generalSamplerIndexing               = 1;
          m_builtinResource.limits.generalVariableIndexing              = 1;
          m_builtinResource.limits.generalConstantMatrixVectorIndexing  = 1;
      }

  };

}