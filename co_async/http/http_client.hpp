#pragma once

#include <co_async/std.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/http/http_status_code.hpp>
#include <co_async/utils/string_utils.hpp>
#include <co_async/system/socket.hpp>
#include <co_async/system/fs.hpp>
#include <co_async/http/uri.hpp>
#include <co_async/http/http_protocol.hpp>
#include <co_async/iostream/socket_stream.hpp>
#include <co_async/iostream/ssl_socket_stream.hpp>
#include <co_async/iostream/cached_stream.hpp>
#include <co_async/threading/future.hpp>
#include <co_async/threading/concurrent_queue.hpp>
#include <co_async/threading/condition_variable.hpp>

namespace co_async {

inline SSLClientTrustAnchor gTrustAnchors;

inline Task<> https_load_ca_certificates() {
    auto path = make_path("/etc/ssl/certs/ca-certificates.crt");
    if (auto s = co_await fs_stat(path); s && s->is_readable()) [[likely]] {
        if (auto content = co_await file_read(path)) [[likely]] {
            gTrustAnchors.add(*content);
        }
    }
}

struct HTTPConnection {
private:
    struct HTTPProtocolFactory {
    protected:
        std::string mHost;
        int mPort;
        std::string mHostName;
        std::string mProxy;
        std::chrono::nanoseconds mTimeout;

    public:
        HTTPProtocolFactory(std::string host, int port,
                            std::string_view hostName, std::string proxy,
                            std::chrono::nanoseconds timeout)
            : mHost(std::move(host)),
              mPort(port),
              mHostName(hostName),
              mProxy(std::move(proxy)),
              mTimeout(timeout) {}

        virtual Task<Expected<std::unique_ptr<HTTPProtocol>>>
        createConnection() = 0;
        virtual ~HTTPProtocolFactory() = default;

        std::string const &hostName() const noexcept {
            return mHostName;
        }
    };

    struct HTTPProtocolFactoryHTTPS : HTTPProtocolFactory {
        using HTTPProtocolFactory::HTTPProtocolFactory;

        Task<Expected<std::unique_ptr<HTTPProtocol>>>
        createConnection() override {
            static char const *const protocols[] = {
                /* "h2", */
                "http/1.1",
            };
            auto sock = std::make_unique<SSLClientSocketStream>(
                co_await co_await SSLClientSocketStream::connect(
                    mHost.c_str(), mPort, gTrustAnchors, protocols, mProxy,
                    mTimeout));
            if (sock->ssl_get_selected_protocol() ==
                "h2") { // todo: seems always false?
                co_return std::make_unique<HTTPProtocolVersion2>(
                    std::move(sock));
            } else {
                co_return std::make_unique<HTTPProtocolVersion11>(
                    std::move(sock));
            }
        }
    };

    struct HTTPProtocolFactoryHTTP : HTTPProtocolFactory {
        using HTTPProtocolFactory::HTTPProtocolFactory;

        Task<Expected<std::unique_ptr<HTTPProtocol>>>
        createConnection() override {
            auto sock = std::make_unique<SocketStream>(
                co_await co_await SocketStream::connect(mHost.c_str(), mPort,
                                                        mProxy, mTimeout));
            co_return std::make_unique<HTTPProtocolVersion11>(std::move(sock));
        }
    };

    std::unique_ptr<HTTPProtocol> mHttp;
    std::unique_ptr<HTTPProtocolFactory> mHttpFactory;

    friend struct HTTPConnectionPool;

    void terminateLifetime() {
        mHttp = nullptr;
        mHttpFactory = nullptr;
    }

    struct RAIIPointerResetter {
        std::unique_ptr<HTTPProtocol> *mHttp;

        RAIIPointerResetter &operator=(RAIIPointerResetter &&) = delete;

        void neverMind() {
            mHttp = nullptr;
        }

        ~RAIIPointerResetter() {
            if (mHttp) [[unlikely]] {
                *mHttp = nullptr;
            }
        }
    };

    void builtinHeaders(HTTPRequest &req) {
        using namespace std::string_literals;
        req.headers.insert("host"s, mHttpFactory->hostName());
        req.headers.insert("user-agent"s, "co_async/0.0.1"s);
        req.headers.insert("accept"s, "*/*"s);
#if CO_ASYNC_ZLIB
        req.headers.insert("accept-encoding"s,
                           "gzip, compress, deflate, br, zstd"s);
#endif
    }

    Task<Expected<void, std::errc>>
    tryWriteRequestAndBody(HTTPRequest const &request, std::string_view body) {
        for (std::size_t n = 0; n < 3; ++n) {
            if (!mHttp) {
                if (auto e = co_await mHttpFactory->createConnection())
                    [[likely]] {
                    mHttp = std::move(*e);
                    mHttp->initClientState();
                } else {
                    continue;
                }
            }
            if (co_await mHttp->writeRequest(request)) {
                if (co_await mHttp->writeBody(body)) {
                    if (co_await mHttp->sock->peekchar()) {
                        co_return {};
                    }
                }
            }
            /* if (co_await mHttp->writeRequest(request) && */
            /*     co_await mHttp->writeBody(body) && */
            /*     co_await mHttp->sock->peekchar()) [[likely]] { */
            /*     co_return {}; */
            /* } */
            mHttp = nullptr;
        }
        co_return Unexpected{std::errc::connection_aborted};
    }

    Task<Expected<void, std::errc>>
    tryWriteRequestAndBodyStream(HTTPRequest const &request,
                                 IStream &bodyStream) {
        CachedStream cachedStream(&bodyStream);
        for (std::size_t n = 0; n < 3; ++n) {
            if (!mHttp) {
                if (auto e = co_await mHttpFactory->createConnection())
                    [[likely]] {
                    mHttp = std::move(*e);
                    mHttp->initClientState();
                } else {
                    continue;
                }
            }
            if (co_await mHttp->writeRequest(request) &&
                co_await mHttp->writeBodyStream(cachedStream) &&
                co_await mHttp->sock->peekchar()) [[likely]] {
                co_return {};
            }
            (void)cachedStream.raw_seek(0);
            mHttp = nullptr;
        }
        co_return Unexpected{std::errc::connection_aborted};
    }

    HTTPConnection(std::unique_ptr<HTTPProtocolFactory> httpFactory)
        : mHttp(nullptr),
          mHttpFactory(std::move(httpFactory)) {}

private:
    static std::tuple<std::string, int>
    parseHostAndPort(std::string_view hostName, int defaultPort) {
        int port = defaultPort;
        auto host = hostName;
        if (auto i = host.rfind(':'); i != host.npos) {
            if (auto portOpt = from_string<int>(host.substr(i + 1)))
                [[likely]] {
                port = *portOpt;
                host.remove_suffix(host.size() - i);
            }
        }
        return {std::string(host), port};
    }

public:
    Expected<void, std::errc> doConnect(std::string_view host,
                                        std::chrono::nanoseconds timeout,
                                        bool followProxy) {
        terminateLifetime();
        if (host.starts_with("https://")) {
            host.remove_prefix(8);
            std::string proxy;
            if (followProxy) [[likely]] {
                if (auto p = std::getenv("https_proxy")) {
                    proxy = p;
                }
            }
            auto [h, p] = parseHostAndPort(host, 443);
            mHttpFactory = std::make_unique<HTTPProtocolFactoryHTTPS>(
                std::move(h), p, host, std::move(proxy), timeout);
            return {};
        } else if (host.starts_with("http://")) {
            host.remove_prefix(7);
            std::string proxy;
            if (followProxy) {
                if (auto p = std::getenv("http_proxy")) {
                    proxy = p;
                }
            }
            auto [h, p] = parseHostAndPort(host, 80);
            mHttpFactory = std::make_unique<HTTPProtocolFactoryHTTP>(
                std::move(h), p, host, std::move(proxy), timeout);
            return {};
        } else [[unlikely]] {
            return Unexpected{std::errc::protocol_not_supported};
        }
    }

    HTTPConnection() = default;

    Task<Expected<void, std::errc>> request(HTTPRequest req,
                                            std::string_view in,
                                            HTTPResponse &res,
                                            std::string &out) {
        builtinHeaders(req);
        RAIIPointerResetter reset(&mHttp);
        co_await co_await tryWriteRequestAndBody(req, in);
        co_await co_await mHttp->readResponse(res);
        co_await co_await mHttp->readBody(out);
        reset.neverMind();
        co_return {};
    }

    Task<Expected<void, std::errc>> request(HTTPRequest req,
                                            std::string_view in,
                                            HTTPResponse &res, OStream &out) {
        builtinHeaders(req);
        RAIIPointerResetter reset(&mHttp);
        co_await co_await tryWriteRequestAndBody(req, in);
        co_await co_await mHttp->readResponse(res);
        co_await co_await mHttp->readBodyStream(out);
        reset.neverMind();
        co_return {};
    }

    Task<Expected<void, std::errc>>
    request(HTTPRequest req, IStream &in, HTTPResponse &res, std::string &out) {
        RAIIPointerResetter reset(&mHttp);
        co_await co_await tryWriteRequestAndBodyStream(req, in);
        co_await co_await mHttp->readResponse(res);
        co_await co_await mHttp->readBody(out);
        reset.neverMind();
        co_return {};
    }

    Task<Expected<void, std::errc>> request(HTTPRequest req, IStream &in,
                                            HTTPResponse &res, OStream &out) {
        builtinHeaders(req);
        RAIIPointerResetter reset(&mHttp);
        co_await co_await tryWriteRequestAndBodyStream(req, in);
        co_await co_await mHttp->readResponse(res);
        co_await co_await mHttp->readBodyStream(out);
        reset.neverMind();
        co_return {};
    }

    Task<Expected<void, std::errc>>
    request(HTTPRequest req, IStream &in,
            FutureReference<HTTPResponse> const &res, OStream &out) {
        builtinHeaders(req);
        RAIIPointerResetter reset(&mHttp);
        co_await co_await tryWriteRequestAndBodyStream(req, in);
        co_await co_await mHttp->readResponse(res);
        co_await co_await mHttp->readBodyStream(out);
        reset.neverMind();
        co_return {};
    }
};

struct HTTPConnectionPool {
private:
    struct alignas(hardware_destructive_interference_size) PoolEntry {
        HTTPConnection mHttp;
        std::atomic_bool mInuse{false};
        bool mValid{false};
        std::chrono::high_resolution_clock::time_point mLastAccess;
        std::string mHostName;
    };

    std::vector<PoolEntry> mPool;
    ConditionVariable mFreeSlot;
    std::chrono::nanoseconds mTimeout;
    std::chrono::nanoseconds mKeepAlive;
    std::chrono::high_resolution_clock::time_point mLastGC;
    bool mFollowProxy;

public:
    struct HTTPConnectionPtr {
    private:
        PoolEntry *mEntry;
        HTTPConnectionPool *mPool;

        explicit HTTPConnectionPtr(PoolEntry *entry,
                                   HTTPConnectionPool *pool) noexcept
            : mEntry(entry),
              mPool(pool) {}

        friend HTTPConnectionPool;

    public:
        HTTPConnectionPtr() noexcept : mEntry(nullptr) {}

        HTTPConnectionPtr(HTTPConnectionPtr &&that) noexcept
            : mEntry(std::exchange(that.mEntry, nullptr)),
              mPool(std::exchange(that.mPool, nullptr)) {}

        HTTPConnectionPtr &operator=(HTTPConnectionPtr &&that) noexcept {
            std::swap(mEntry, that.mEntry);
            std::swap(mPool, that.mPool);
            return *this;
        }

        ~HTTPConnectionPtr() {
            if (mEntry) {
                mEntry->mLastAccess = std::chrono::high_resolution_clock::now();
                mEntry->mInuse.store(false, std::memory_order_release);
                mPool->mFreeSlot.notify_one();
            }
        }

        HTTPConnection &operator*() const noexcept {
            return mEntry->mHttp;
        }

        HTTPConnection *operator->() const noexcept {
            return &mEntry->mHttp;
        }
    };

    explicit HTTPConnectionPool(
        std::size_t poolSize = 256,
        std::chrono::nanoseconds timeout = std::chrono::seconds(5),
        std::chrono::nanoseconds keepAlive = std::chrono::minutes(3),
        bool followProxy = true)
        : mPool(poolSize),
          mTimeout(timeout),
          mKeepAlive(keepAlive),
          mFollowProxy(followProxy) {}

private:
    Expected<Expected<HTTPConnectionPtr, std::errc>>
    lookForFreeSlot(bool exactReuse, std::string_view host) /* MT-safe */ {
        for (auto &entry: mPool) {
            bool expected = false;
            if (entry.mInuse.compare_exchange_strong(
                    expected, true, std::memory_order_acq_rel)) {
                if (!entry.mValid) {
                    if (auto e =
                            entry.mHttp.doConnect(host, mTimeout, mFollowProxy);
                        e.has_error()) [[unlikely]] {
                        return Unexpected{e.error()};
                    }
                    entry.mHostName = host;
                    entry.mLastAccess =
                        std::chrono::high_resolution_clock::now();
                    entry.mValid = true;
                } else {
                    if (entry.mHostName == host) {
                        entry.mLastAccess =
                            std::chrono::high_resolution_clock::now();
                    } else {
                        if (exactReuse) {
                            entry.mInuse.store(false,
                                               std::memory_order_release);
                            continue;
                        } else {
                            if (auto e = entry.mHttp.doConnect(host, mTimeout,
                                                               mFollowProxy);
                                e.has_error()) [[unlikely]] {
                                return Unexpected{e.error()};
                            }
                            entry.mHostName = host;
                            entry.mLastAccess =
                                std::chrono::high_resolution_clock::now();
                        }
                    }
                }
                return HTTPConnectionPtr(&entry, this);
            }
        }
        return Unexpected{};
    }

    void garbageCollect() /* MT-safe */ {
        auto now = std::chrono::high_resolution_clock::now();
        if ((now - mLastGC) * 2 > mKeepAlive) {
            for (auto &entry: mPool) {
                bool expected = false;
                if (entry.mInuse.compare_exchange_strong(
                        expected, true, std::memory_order_acq_rel)) {
                    if (entry.mValid && now - entry.mLastAccess > mKeepAlive) {
                        entry.mHttp.terminateLifetime();
                        entry.mHostName.clear();
                        entry.mValid = false;
                    }
                    entry.mInuse.store(false, std::memory_order_release);
                }
            }
            mLastGC = now;
        }
    }

public:
    Task<Expected<HTTPConnectionPtr, std::errc>>
    connect(std::string_view host) /* MT-safe */ {
    again:
        garbageCollect();
        if (auto conn = lookForFreeSlot(true, host)) {
            co_return std::move(*conn);
        } else if (auto conn = lookForFreeSlot(false, host)) {
            co_return std::move(*conn);
        }
#if CO_ASYNC_DEBUG
        std::cerr << "WARNING: connection pool run out of free slot\n";
#endif
        co_await mFreeSlot;
        goto again;
    }
};

} // namespace co_async
