#include <co_async/awaiter/task.hpp>
#include <co_async/generic/io_context.hpp>
#include <co_async/iostream/file_stream.hpp>
#include <co_async/iostream/stream_base.hpp>
#include <co_async/platform/fs.hpp>

namespace co_async {
namespace {
struct FileStream : Stream {
    Task<Expected<std::size_t>> raw_read(std::span<char> buffer) override {
        co_return co_await fs_read(mFile, buffer, co_await co_cancel);
    }

    Task<Expected<std::size_t>>
    raw_write(std::span<char const> buffer) override {
        co_return co_await fs_write(mFile, buffer, co_await co_cancel);
    }

    Task<> raw_close() override {
        (co_await fs_close(std::move(mFile))).value_or();
    }

    FileHandle release() noexcept {
        return std::move(mFile);
    }

    FileHandle &get() noexcept {
        return mFile;
    }

    explicit FileStream(FileHandle file) : mFile(std::move(file)) {}

private:
    FileHandle mFile;
};
} // namespace

Task<Expected<OwningStream>> file_open(std::filesystem::path path,
                                       OpenMode mode) {
    co_return make_stream<FileStream>(co_await co_await fs_open(path, mode));
}

OwningStream file_from_handle(FileHandle handle) {
    return make_stream<FileStream>(std::move(handle));
}

Task<Expected<String>> file_read(std::filesystem::path path) {
    auto file = co_await co_await file_open(path, OpenMode::Read);
    co_return co_await file.getall();
}

Task<Expected<>> file_write(std::filesystem::path path,
                            std::string_view content) {
    auto file = co_await co_await file_open(path, OpenMode::Write);
    co_await co_await file.puts(content);
    co_await co_await file.flush();
    co_return {};
}

Task<Expected<>> file_append(std::filesystem::path path,
                             std::string_view content) {
    auto file = co_await co_await file_open(path, OpenMode::Append);
    co_await co_await file.puts(content);
    co_await co_await file.flush();
    co_return {};
}
} // namespace co_async
