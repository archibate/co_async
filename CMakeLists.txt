cmake_minimum_required(VERSION 3.28)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(co_async LANGUAGES CXX)

add_compile_options($<$<COMPILE_LANG_AND_ID:CXX,MSVC>:-utf-8>)
add_compile_options($<$<COMPILE_LANG_AND_ID:CXX,Clang>:-fimplicit-modules$<SEMICOLON>-fimplicit-module-maps$<SEMICOLON>-stdlib=libc++>)
add_link_options($<$<COMPILE_LANG_AND_ID:CXX,Clang>:-fimplicit-modules$<SEMICOLON>-fimplicit-module-maps$<SEMICOLON>-stdlib=libc++>)

add_library(co_async OBJECT)
file(GLOB_RECURSE co_async_sources CONFIGURE_DEPENDS co_async/*.cpp)
target_sources(co_async PUBLIC FILE_SET CXX_MODULES FILES ${co_async_sources})
if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    find_library(URING_LIBRARY NAMES uring REQUIRED)
    message(STATUS "Found liburing: ${URING_LIBRARY}")
    target_link_libraries(co_async PUBLIC ${URING_LIBRARY})
endif()

add_executable(main main.cpp)
target_link_libraries(main PRIVATE co_async)
