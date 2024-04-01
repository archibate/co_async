export module co_async:iostream.file_stream;

import std;
import :system.fs;
import :awaiter.task;
import :iostream.stream_base;

namespace co_async {

export struct FileBuf {
    Task<std::size_t> read(std::span<char> buffer) {
        return fs_read(mFile, buffer);
    }

    Task<std::size_t> write(std::span<char const> buffer) {
        return fs_write(mFile, buffer);
    }

    FileHandle release() noexcept {
        return std::move(mFile);
    }

    explicit FileBuf(FileHandle file) : mFile(std::move(file)) {}

private:
    FileHandle mFile;
};

export using FileIStream = IStream<FileBuf>;
export using FileOStream = OStream<FileBuf>;
export using FileStream = IOStream<FileBuf>;

} // namespace co_async
