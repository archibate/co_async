#set(CO_ASYNC_DEBUG ON)
#set(CO_ASYNC_SAFERET ON)
#set(CO_ASYNC_ALLOC ON)
#set(CO_ASYNC_PERF ON)
#set(CO_ASYNC_EXCEPT ON)
#set(CO_ASYNC_ZLIB ON)
#set(CO_ASYNC_STEAL ON)
#set(CO_ASYNC_CUDA ON)
#set(CO_ASYNC_DIRECT ON)
#set(CO_ASYNC_INVALFIX ON)
#set(CO_ASYNC_NATIVE ON)
# set(CO_ASYNC_WARN ON)
#set(CO_ASYNC_FIND_LIBURING ON)
#set(CO_ASYNC_FIND_BEARSSL ON)
#set(CO_ASYNC_JEMALLOC ON)

cmake_minimum_required(VERSION 3.16)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (CMAKE_BUILD_TYPE MATCHES "Debug" AND NOT DEFINED CO_ASYNC_DEBUG)
    set(CO_ASYNC_DEBUG ON)
endif()

project(co_async LANGUAGES CXX)

if (CO_ASYNC_CUDA)
    enable_language(CUDA)
endif()

if (PROJECT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(CXXFLAGS)
    if (MSVC)
        list(APPEND CXXFLAGS -utf-8 -Zc:preprocessor -permissive- -EHsc)
        add_definitions(-DNOMINMAX)
        if (CO_ASYNC_WARN)
            list(APPEND CXXFLAGS -W4)
        endif()
    else()
        if (CO_ASYNC_WARN)
            list(APPEND CXXFLAGS -Wall -Wextra -Wno-interference-size -Werror=return-type
            -Werror=unused-result -Werror=uninitialized -Werror=maybe-uninitialized
            -Wno-unused-parameter -Wunused-but-set-variable -Wunused-but-set-parameter -Wunused-function
            -Wunused-const-variable -Werror=use-after-free=3 -Werror=invalid-memory-model -Wunused-value
            -Wexceptions -Werror=missing-declarations -Werror=missing-field-initializers
            -Wparentheses -Wconversion -Werror=float-conversion -Wsign-conversion -Wsign-compare -Wno-terminate
            -Werror=delete-non-virtual-dtor -Werror=suggest-override -Wsign-promo -Wold-style-cast
            -Wrange-loop-construct -Werror=reorder -Werror=class-memaccess -Werror=narrowing
            -Wdeprecated-enum-enum-conversion -Werror=deprecated-copy-dtor -Werror=deprecated-copy
            -Wzero-as-null-pointer-constant -Wplacement-new=2 -Wsuggest-final-types -Wsuggest-final-methods
            -Werror=conversion-null -Werror=mismatched-new-delete -Werror=address -Wlogical-op -Wlogical-not-parentheses
            -Wattributes -Wmissing-declarations -Werror=multichar -Werror=overflow -Werror=restrict -Werror=vla
            -Wstrict-aliasing=1 -Werror=string-compare -Werror=stringop-overflow=2 -Werror=shift-overflow
            -Wsuggest-attribute=const -Wsuggest-attribute=pure -Wsuggest-attribute=noreturn -Werror=alloca
            -Werror=array-bounds -Werror=bool-compare -Werror=bool-operation -Werror=zero-length-bounds
            -Werror=div-by-zero -Wno-shadow -Werror=format -Werror=pointer-arith -Werror=write-strings
            -Werror=dangling-pointer=2 -Werror=return-local-addr -Wempty-body -Wimplicit-fallthrough
            -Wswitch -Wno-unknown-warning-option)
        elseif(CO_ASYNC_DEBUG)
            list(APPEND CXXFLAGS -Wall -Wextra -Wno-interference-size -Wreturn-type -Wno-unused-parameter )
        endif()
        if (CO_ASYNC_DEBUG)
            list(APPEND CXXFLAGS -Wno-exceptions)
        else()
            if (CMAKE_BUILD_TYPE MATCHES "[Rr][Ee][Ll][Ee][Aa][Ss][Ee]")
                list(APPEND CXXFLAGS -flto)
            endif()
        endif()
    endif()
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 10.0.0 AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 11.0.0)
        #add_definitions(-D__cplusplus=202002L)
        list(APPEND CXXFLAGS -fcoroutines)
        if (IS_DIRECTORY /usr/include/c++/10)
            include_directories($<$<COMPILE_LANG_AND_ID:CXX,GNU>:/usr/include/c++/10>)
        endif()
        if (CO_ASYNC_WARN)
            list(APPEND CXXFLAGS -Wno-zero-as-null-pointer-constant -Wno-unused-const-variable)
        endif()
        list(APPEND CXXFLAGS -Wno-attributes)
        if (CO_ASYNC_DEBUG)
            list(APPEND CXXFLAGS -Wno-terminate)
        endif()
    else()
        if (CO_ASYNC_WARN)
            list(APPEND CXXFLAGS -Werror)
        endif()
    endif()
endif()

include(CheckCXXCompilerFlag)
foreach (flag IN LISTS CXXFLAGS)
    set(name ${flag})
    # replace - and = with _ and remove _ prefix and uppercase
    string(REGEX REPLACE "[-=:]" "_" name ${name})
    string(TOUPPER ${name} name)
    string(REGEX REPLACE "^_" "" name ${name})
    # check if the compiler supports the warning
    check_cxx_compiler_flag(${flag} HAS_${name})
    if (HAS_${name})
        add_compile_options(${flag})
    endif()
endforeach()

# check if they can compile a sample source
include(CheckCXXSourceCompiles)
set(CMAKE_REQUIRED_FLAGS ${CXXFLAGS})
check_cxx_source_compiles("
#include <coroutine>

int main() {
    std::coroutine_handle<> c;
    (void)c;
    return 0;
}" HAS_COROUTINE)
if (NOT HAS_COROUTINE)
    message(FATAL_ERROR "Your compiler doesn't support coroutine")
endif()

# check if the compiler support symmetric coroutines
check_cxx_source_compiles("
#include <coroutine>

struct awaiter {
    bool await_ready() const { return false; }
    std::coroutine_handle<> await_suspend(std::coroutine_handle<>) const {
        return std::noop_coroutine();
    }
    void await_resume() const {}
};

struct symmetric {
    struct promise_type {
        symmetric get_return_object() { return symmetric{}; }
        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() {}
    };
};

symmetric task() {
    co_await awaiter{};
    co_return;
}

int main() {
    task();
    return 0;
}" HAS_SYMMETRIC_COROUTINE)
if (NOT HAS_SYMMETRIC_COROUTINE)
    message(FATAL_ERROR "Your compiler doesn't support symmetric coroutine transfer")
endif()


add_library(co_async OBJECT)
file(GLOB_RECURSE interface_sources co_async/*.hpp)
file(GLOB_RECURSE implementation_sources co_async/*.cpp)
target_include_directories(co_async PUBLIC .)
target_sources(co_async PRIVATE ${implementation_sources})
if (NOT implementation_sources)
    target_sources(co_async PRIVATE scripts/dummy.cpp)
endif()
if (CO_ASYNC_DEBUG)
     target_compile_definitions(co_async PUBLIC CO_ASYNC_DEBUG)
endif()
if (CO_ASYNC_SAFERET)
    target_compile_definitions(co_async PUBLIC CO_ASYNC_SAFERET)
endif()
if (CO_ASYNC_ALLOC)
    target_compile_definitions(co_async PUBLIC CO_ASYNC_ALLOC)
endif()
if (CO_ASYNC_PERF)
    target_compile_definitions(co_async PUBLIC CO_ASYNC_PERF)
endif()
if (CO_ASYNC_EXCEPT)
    target_compile_definitions(co_async PUBLIC CO_ASYNC_EXCEPT)
endif()
if (CO_ASYNC_ZLIB)
    target_compile_definitions(co_async PUBLIC CO_ASYNC_ZLIB)
    # target_link_libraries(co_async PUBLIC z)
    find_package(ZLIB REQUIRED)
    target_link_libraries(co_async PUBLIC ZLIB::ZLIB)
endif()
if (CO_ASYNC_STEAL)
    target_compile_definitions(co_async PUBLIC CO_ASYNC_STEAL)
endif()
if (CO_ASYNC_CUDA)
    target_compile_definitions(co_async PUBLIC CO_ASYNC_CUDA)
    find_package(CUDA REQUIRED)
    target_link_libraries(co_async PUBLIC CUDA::CUDA)
endif()
if (CO_ASYNC_DIRECT)
    target_compile_definitions(co_async PUBLIC CO_ASYNC_DIRECT)
endif()
if (CO_ASYNC_INVALFIX)
    target_compile_definitions(co_async PUBLIC CO_ASYNC_INVALFIX)
endif()
if (CO_ASYNC_NATIVE)
    target_compile_definitions(co_async PUBLIC CO_ASYNC_NATIVE)
    target_compile_options(co_async PRIVATE -march=native)
endif()

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
    if (CO_ASYNC_FIND_LIBURING)
        include(FindPkgConfig)
        pkg_check_modules(LIBURING liburing)
        if (NOT LIBURING_FOUND)
            find_path(LIBURING_INCLUDE_DIR NAMES liburing.h)
            find_library(LIBURING_LIBRARY NAMES liburing.a)
            if (NOT LIBURING_INCLUDE_DIR OR NOT LIBURING_LIBRARY)
                message(FATAL_ERROR "liburing not found")
            endif()
            set(LIBURING_LIBRARIES ${LIBURING_LIBRARY})
            set(LIBURING_INCLUDE_DIRS ${LIBURING_INCLUDE_DIR})
        endif()
        target_link_libraries(co_async PUBLIC ${LIBURING_LIBRARIES})
        target_include_directories(co_async PUBLIC ${LIBURING_INCLUDE_DIRS})
    else()
        target_sources(co_async PRIVATE 3rd/liburing/liburing.cpp)
        target_include_directories(co_async PUBLIC 3rd/liburing/include)
    endif()
endif()
target_include_directories(co_async PUBLIC 3rd/hashlib/include)

if (CO_ASYNC_FIND_BEARSSL)
    find_package(BearSSL QUIET)
    if (TARGET BearSSL::bearssl)
        target_link_libraries(co_async PUBLIC BearSSL::bearssl)
    else()
        find_path(BEARSSL_INCLUDE_DIR NAMES bearssl.h)
        find_library(BEARSSL_LIBRARY NAMES libbearssl.so)
        if (NOT BEARSSL_INCLUDE_DIR OR NOT BEARSSL_LIBRARY)
            message(FATAL_ERROR "BearSSL not found")
        endif()
        target_link_libraries(co_async PUBLIC ${BEARSSL_LIBRARY})
        target_include_directories(co_async PUBLIC ${BEARSSL_INCLUDE_DIR})
    endif()
else()
    target_include_directories(co_async PUBLIC 3rd/bearssl/include)
endif()

find_package(Threads REQUIRED)
target_link_libraries(co_async PUBLIC Threads::Threads)

find_package(TBB REQUIRED COMPONENTS tbb REQUIRED)
target_link_libraries(co_async PUBLIC TBB::tbb)

if (CO_ASYNC_JEMALLOC)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(JEMALLOC jemalloc)
    pkg_search_module(JEMALLOC REQUIRED jemalloc)
    include_directories(${JEMALLOC_INCLUDE_DIRS})
    target_link_libraries(co_async PUBLIC ${JEMALLOC_LIBRARIES})
endif()

if (PROJECT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    file(GLOB example_sources examples/*.cpp)
    foreach(path ${example_sources})
        get_filename_component(name ${path} NAME_WE)
        add_executable(${name} ${path})
        target_link_libraries(${name} PRIVATE co_async)
    endforeach()
endif()

add_custom_target(format
    COMMAND clang-format -i ${implementation_sources} ${interface_sources}
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
