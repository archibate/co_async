#pragma once

#include <zlib.h>
#include <co_async/std.hpp>
#include <co_async/system/error_handling.hpp>
#include <co_async/system/fs.hpp>
#include <co_async/system/socket.hpp>
#include <co_async/system/system_loop.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/utils/string_utils.hpp>
#include <co_async/iostream/stream_base.hpp>

namespace co_async {

struct ZlibDecompressStreamRaw : StreamRaw {
public:
    explicit ZlibDecompressStreamRaw(BorrowedStream &stream) : mStream(stream) {
        int ret = deflateInit(&mDeflate, Z_DEFAULT_COMPRESSION);
        if (ret != Z_OK) {
            throw std::runtime_error("deflateInit failed with error code " +
                                     to_string(ret));
        }
    }

    BorrowedStream &base() const noexcept {
        return mStream;
    }

    ZlibDecompressStreamRaw(ZlibDecompressStreamRaw &&) = delete;

    ~ZlibDecompressStreamRaw() {
        deflateEnd(&mDeflate);
    }

    Task<Expected<std::size_t, std::errc>> raw_read(std::span<char> out_buffer) override {
        if (mStream.bufempty()) {
            if (!co_await mStream.fillbuf()) [[unlikely]] {
                co_return 0;
            }
        }
        auto bufSpan = mStream.peekbuf();

        mDeflate.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(bufSpan.data()));
        mDeflate.avail_in = static_cast<unsigned int>(bufSpan.size());

        mDeflate.next_out = reinterpret_cast<Bytef *>(out_buffer.data());
        mDeflate.avail_out = static_cast<unsigned int>(out_buffer.size());

        int flush = Z_NO_FLUSH;
        do {
            int ret = deflate(&mDeflate, flush);
            if (ret == Z_STREAM_ERROR) {
#if CO_ASYNC_DEBUG
                std::cerr << "WARNING: deflate failed with error: " + to_string(mDeflate.msg) +
                                 "\n";
#endif
                co_return 0;
            }
            if (out_buffer.size() - mDeflate.avail_out > 0) {
                co_return out_buffer.size() - mDeflate.avail_out;
            }
        } while (mDeflate.avail_out == 0);
        co_return 0;
    }

    void zlib_reset() {
        int ret = deflateReset(&mDeflate);
        if (ret != Z_OK) {
            throw std::runtime_error("deflateReset failed with error code " +
                                     to_string(ret));
        }
    }

protected:
    BorrowedStream &mStream;
    z_stream mDeflate;
};

} // namespace co_async
