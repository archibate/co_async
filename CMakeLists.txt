cmake_minimum_required(VERSION 3.18)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
set(CMAKE_CXX_STANDARD 20)

project(main LANGUAGES CXX)

add_executable(main main.cpp)
target_include_directories(main PUBLIC .)
target_link_libraries(main PUBLIC uring)
# target_compile_definitions(main PUBLIC -DCO_ASYNC_PERF)
target_compile_definitions(main PUBLIC -DCO_ASYNC_DEBUG)
target_compile_options(main PRIVATE $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:-utf-8>)
