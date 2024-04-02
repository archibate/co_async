cmake_minimum_required(VERSION 3.28)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(co_async LANGUAGES CXX)

include(cmake/clang_std_modules.cmake)
add_compile_options($<$<COMPILE_LANG_AND_ID:CXX,MSVC>:-utf-8>)

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
endif()

add_library(co_async OBJECT)
enable_std_modules(co_async)
file(GLOB_RECURSE co_async_sources CONFIGURE_DEPENDS co_async/*.cpp)
target_sources(co_async PUBLIC FILE_SET CXX_MODULES FILES ${co_async_sources})

add_executable(main main.cpp)
enable_std_modules(main)
target_link_libraries(main PRIVATE co_async)
