cmake_minimum_required(VERSION 3.15)

project(raynet LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.c")

add_executable(raynet ${SOURCES})

target_compile_features(raynet PRIVATE c_std_11)
target_include_directories(raynet PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_compile_definitions(raynet PRIVATE RAYNET_PROJECT_DIR=\"${CMAKE_SOURCE_DIR}\")

if(WIN32)
    target_compile_definitions(raynet PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
endif()