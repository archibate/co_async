/*{module;}*/

#ifdef __linux__
#include <unistd.h>
#endif

#pragma once/*{export module co_async:system.socket;}*/

#include <co_async/std.hpp>/*{import std;}*/

#ifdef __linux__
#include <co_async/system/error_handling.hpp>/*{import :system.error_handling;}*/
#include <co_async/system/fs.hpp>/*{import :system.fs;}*/
#include <co_async/awaiter/task.hpp>/*{import :awaiter.task;}*/

namespace co_async {

/*[export]*/ struct PipeHandlePair {
    FileHandle mReader;
    FileHandle mWriter;

    FileHandle reader() {
#if CO_ASYNC_DEBUG
        if (!mReader) [[unlikely]] {
            throw std::invalid_argument(
                "PipeHandlePair::reader() can only be called once");
        }
#endif
        return std::move(mReader);
    }

    FileHandle writer() {
#if CO_ASYNC_DEBUG
        if (!mWriter) [[unlikely]] {
            throw std::invalid_argument(
                "PipeHandlePair::writer() can only be called once");
        }
#endif
        return std::move(mWriter);
    }
};

/*[export]*/ inline Task<PipeHandlePair> make_pipe() {
    int p[2];
    checkError(pipe2(p, 0));
    co_return {FileHandle(p[0]), FileHandle(p[1])};
}

}
#endif
