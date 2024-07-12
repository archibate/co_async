#include <co_async/awaiter/task.hpp>
#include <co_async/iostream/stream_base.hpp>
#include <co_async/iostream/zlib_stream.hpp>
#include <co_async/platform/platform_io.hpp>
#include <co_async/utils/expected.hpp>
#if CO_ASYNC_ZLIB
# include <zlib.h>
#endif

namespace co_async {
#if CO_ASYNC_ZLIB
// borrowed from: https://github.com/intel/zlib/blob/master/examples/zpipe.c
Task<Expected<>> zlib_inflate(BorrowedStream &source, BorrowedStream &dest) {
    /* decompress */
    int ret;
    unsigned have;
    z_stream strm;
    constexpr std::size_t chunk = kStreamBufferSize;
    unsigned char in[chunk];
    unsigned char out[chunk];
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit(&strm);
    if (ret != Z_OK) [[unlikely]] {
# if CO_ASYNC_DEBUG
        std::cerr << "WARNING: inflateInit returned error\n";
# endif
        co_return std::errc::not_enough_memory;
    }
    do {
        if (auto e = co_await source.read(std::span<char>((char *)in, chunk));
            e.has_error()) [[unlikely]] {
# if CO_ASYNC_DEBUG
            std::cerr << "WARNING: inflate source read failed with error\n";
# endif
            (void)inflateEnd(&strm);
            co_return CO_ASYNC_ERROR_FORWARD(e);
        } else {
            strm.avail_in = *e;
        }
        if (strm.avail_in == 0) {
            break;
        }
        strm.next_in = in;
        do {
            strm.avail_out = chunk;
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);
            assert(ret != Z_STREAM_ERROR);
            switch (ret) {
            case Z_NEED_DICT:  [[fallthrough]];
            case Z_DATA_ERROR: [[fallthrough]];
            case Z_MEM_ERROR:  (void)inflateEnd(&strm);
# if CO_ASYNC_DEBUG
                std::cerr << "WARNING: inflate error: " + std::to_string(ret) +
                                 ": " + std::string(strm.msg) + "\n";
# endif
                co_return std::errc::io_error;
            }
            have = chunk - strm.avail_out;
            if (auto e = co_await dest.putspan(
                    std::span<char const>((char const *)out, have));
                e.has_error()) [[unlikely]] {
# if CO_ASYNC_DEBUG
                std::cerr << "WARNING: inflate dest write failed with error\n";
# endif
                (void)inflateEnd(&strm);
                co_return CO_ASYNC_ERROR_FORWARD(e);
            }
        } while (strm.avail_out == 0);
    } while (ret != Z_STREAM_END);
    (void)inflateEnd(&strm);
    if (ret != Z_STREAM_END) [[unlikely]] {
# if CO_ASYNC_DEBUG
        std::cerr << "WARNING: inflate got unexpected end of file\n";
# endif
        co_return std::errc::io_error;
    }
    co_return {};
}

Task<Expected<>> zlib_deflate(BorrowedStream &source, BorrowedStream &dest) {
    /* compress */
    int ret, flush;
    unsigned have;
    z_stream strm;
    constexpr std::size_t chunk = kStreamBufferSize;
    unsigned char in[chunk];
    unsigned char out[chunk];
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = deflateInit(&strm, Z_DEFAULT_COMPRESSION);
    if (ret != Z_OK) [[unlikely]] {
# if CO_ASYNC_DEBUG
        std::cerr << "WARNING: deflateInit returned error\n";
# endif
        co_return std::errc::not_enough_memory;
    }
    do {
        if (auto e = co_await source.read(std::span<char>((char *)in, chunk));
            e.has_error()) [[unlikely]] {
# if CO_ASYNC_DEBUG
            std::cerr << "WARNING: deflate source read failed with error\n";
# endif
            (void)deflateEnd(&strm);
            co_return CO_ASYNC_ERROR_FORWARD(e);
        } else {
            strm.avail_in = *e;
        }
        if (strm.avail_in == 0) {
            flush = Z_FINISH;
        } else {
            flush = Z_NO_FLUSH;
        }
        strm.next_in = in;
        do {
            strm.avail_out = chunk;
            strm.next_out = out;
            ret = deflate(&strm, flush);
            assert(ret != Z_STREAM_ERROR);
            have = chunk - strm.avail_out;
            if (auto e = co_await dest.putspan(
                    std::span<char const>((char const *)out, have));
                e.has_error()) [[unlikely]] {
# if CO_ASYNC_DEBUG
                std::cerr << "WARNING: deflate dest write failed with error\n";
# endif
                (void)deflateEnd(&strm);
                co_return CO_ASYNC_ERROR_FORWARD(e);
            }
        } while (strm.avail_out == 0);
        assert(strm.avail_in == 0); /* all input will be used */
    } while (flush != Z_FINISH);
    (void)deflateEnd(&strm);
    co_return {};
}
#else
Task<Expected<>> zlib_inflate(BorrowedStream &source, BorrowedStream &dest) {
    co_return std::errc::function_not_supported;
}

Task<Expected<>> zlib_deflate(BorrowedStream &source, BorrowedStream &dest) {
    co_return std::errc::function_not_supported;
}
#endif
} // namespace co_async
