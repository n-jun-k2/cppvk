cmake_minimum_required(VERSION 3.8)

project(CppVk)

set(CppVkLIBS
    vulkan-1
    glfw3
)

file (
    GLOB_RECURSE
    SOURCES
    ${PROJECT_SOURCE_DIR}/src/*.cpp
    ${PROJECT_SOURCE_DIR}/include/*.cpp
)

include_directories(
    $ENV{VULKAN_SDK}/include
    ${PROJECT_SOURCE_DIR}/include
)

link_directories(
    ${PROJECT_SOURCE_DIR}/lib
    $ENV{VULKAN_SDK}/Lib
)

add_executable(
    ${PROJECT_NAME}
    ${SOURCES}
)

target_link_libraries(
    ${PROJECT_NAME} PRIVATE
    ${CppVkLIBS}
)

set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/ignore:4099")

target_compile_options(
    ${PROJECT_NAME} PRIVATE
    "/MD"
)

target_compile_features(
    ${PROJECT_NAME} PRIVATE
    cxx_std_17
)