/*{module;}*/

#include <zlib.h>

#pragma once /*{export module co_async:iostream.zlib_stream;}*/

#include <co_async/std.hpp>/*{import std;}*/
#include <co_async/system/error_handling.hpp>/*{import :system.error_handling;}*/
#include <co_async/system/fs.hpp>            /*{import :system.fs;}*/
#include <co_async/system/socket.hpp>        /*{import :system.socket;}*/
#include <co_async/system/system_loop.hpp>   /*{import :system.system_loop;}*/
#include <co_async/awaiter/task.hpp>         /*{import :awaiter.task;}*/
#include <co_async/utils/string_utils.hpp>   /*{import :utils.string_utils;}*/
#include <co_async/iostream/stream_base.hpp>/*{import :iostream.stream_base;}*/

namespace co_async {

template <class BaseStream>
struct ZlibDecompressStreamRaw {
protected:
    z_stream zs;
    BaseStream &base;

public:
    explicit ZlibDecompressStreamRaw(BaseStream &base) : base(base) {
        int ret = deflateInit(&zs, Z_DEFAULT_COMPRESSION);
        if (ret != Z_OK) {
            throw std::runtime_error("deflateInit failed with error code " + to_string(ret));
        }
    }

    ZlibDecompressStreamRaw(ZlibDecompressStreamRaw &&) = delete;

    ~ZlibDecompressStreamRaw() {
        deflateEnd(&zs);
    }

    Task<std::size_t> raw_read(std::span<char> out_buffer) {
        auto bufSpan = base.rdbuf();
        if (!bufSpan.size() && !co_await base.fillbuf()) {
            co_return 0;
        }

        zs.next_in = reinterpret_cast<Bytef *>(bufSpan.data());
        zs.avail_in = static_cast<unsigned int>(bufSpan.size());

        zs.next_out = reinterpret_cast<Bytef *>(out_buffer.data());
        zs.avail_out = static_cast<unsigned int>(out_buffer.size());

        int flush = Z_NO_FLUSH;
        do {
            int ret = deflate(&zs, flush);
            if (ret == Z_STREAM_ERROR) {
#if CO_ASYNC_DEBUG
                std::cerr << "deflate failed with error: " + to_string(zs.msg) + "\n";
#endif
                co_return 0;
            }
            if (out_buffer.size() - zs.avail_out > 0) {
                co_return out_buffer.size() - zs.avail_out;
            }
        } while (zs.avail_out == 0);
        co_return 0;
    }

    Task<> reset() {
        int ret = deflateReset(&zs);
        if (ret != Z_OK) {
            throw std::runtime_error("deflateReset failed with error code " + to_string(ret));
        }
    }
};

}
