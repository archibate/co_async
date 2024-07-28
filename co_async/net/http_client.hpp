#pragma once
#include <co_async/std.hpp>
#include <co_async/awaiter/task.hpp>
#include <co_async/generic/condition_variable.hpp>
#include <co_async/generic/mutex.hpp>
#include <co_async/generic/when_any.hpp>
#include <co_async/iostream/cached_stream.hpp>
#include <co_async/iostream/file_stream.hpp>
#include <co_async/iostream/pipe_stream.hpp>
#include <co_async/iostream/socket_stream.hpp>
#include <co_async/iostream/ssl_socket_stream.hpp>
#include <co_async/net/http_protocol.hpp>
#include <co_async/net/http_string_utils.hpp>
#include <co_async/net/uri.hpp>
#include <co_async/platform/fs.hpp>
#include <co_async/platform/socket.hpp>
#include <co_async/utils/string_utils.hpp>

namespace co_async {
struct HTTPConnection {
private:
    struct HTTPProtocolFactory {
    protected:
        std::string mHost;
        int mPort;
        std::string mHostName;
        std::string mProxy;
        std::chrono::steady_clock::duration mTimeout;

    public:
        HTTPProtocolFactory(std::string host, int port,
                            std::string_view hostName, std::string proxy,
                            std::chrono::steady_clock::duration timeout)
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

        static PImpl<SSLClientTrustAnchor> &trustAnchors() {
            static PImpl<SSLClientTrustAnchor> instance;
            return instance;
        }

        Task<Expected<std::unique_ptr<HTTPProtocol>>>
        createConnection() override {
            static CallOnce taInitializeOnce;
            static char const *const protocols[] = {
                /* "h2", */
                "http/1.1",
            };
            if (auto locked = co_await taInitializeOnce.call_once()) {
                auto path = make_path("/etc/ssl/certs/ca-certificates.crt");
                auto content = co_await co_await file_read(path);
                co_await trustAnchors().add(content);
                locked.set_ready();
            }
            auto sock = co_await co_await ssl_connect(mHost.c_str(), mPort,
                                                      trustAnchors(), protocols,
                                                      mProxy, mTimeout);
            co_return std::make_unique<HTTPProtocolVersion11>(std::move(sock));
        }
    };

    struct HTTPProtocolFactoryHTTP : HTTPProtocolFactory {
        using HTTPProtocolFactory::HTTPProtocolFactory;

        Task<Expected<std::unique_ptr<HTTPProtocol>>>
        createConnection() override {
            auto sock = co_await co_await tcp_connect(mHost.c_str(), mPort,
                                                      mProxy, mTimeout);
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
#if CO_ASYNC_DEBUG
        if (!mHttpFactory) [[unlikely]] {
            throw std::logic_error("http factory not initialized");
        }
#endif
        req.headers.insert("host"_s, String{mHttpFactory->hostName()});
        req.headers.insert("user-agent"_s, "co_async/0.0.1"_s);
        req.headers.insert("accept"_s, "*/*"_s);
#if CO_ASYNC_ZLIB
        req.headers.insert("accept-encoding"_s, "deflate, gzip"_s);
#else
        req.headers.insert("accept-encoding"_s, "gzip"_s);
#endif
    }

    Task<Expected<>> tryWriteRequestAndBody(HTTPRequest const &request,
                                            std::string_view body) {
        if (!mHttp)
            mHttp = co_await co_await mHttpFactory->createConnection();
        co_await co_await mHttp->writeRequest(request);
        co_await co_await mHttp->writeBody(body);
        co_return {};
#if 0
        std::error_code ec;
        for (std::size_t n = 0; n < 3; ++n) {
            if (!mHttp) {
                if (auto e = co_await mHttpFactory->createConnection())
                    [[likely]] {
                    mHttp = std::move(*e);
                    mHttp->initClientState();
                } else {
                    ec = e.error();
                    continue;
                }
            }
            if (co_await mHttp->writeRequest(request) &&
                co_await mHttp->writeBody(body) &&
                co_await mHttp->sock.peekchar()) [[likely]] {
                co_return {};
            }
            mHttp = nullptr;
        }
        co_return ec;
#endif
    }

    /* Task<Expected<>> */
    /* tryWriteRequestAndBodyStream(HTTPRequest const &request, */
    /*                              BorrowedStream &bodyStream) { */
    /*     auto cachedStream = make_stream<CachedStream>(bodyStream); */
    /*     for (std::size_t n = 0; n < 3; ++n) { */
    /*         if (!mHttp) { */
    /*             if (auto e = co_await mHttpFactory->createConnection()) */
    /*                 [[likely]] { */
    /*                 mHttp = std::move(*e); */
    /*                 mHttp->initClientState(); */
    /*             } else { */
    /*                 continue; */
    /*             } */
    /*         } */
    /*         if (co_await mHttp->writeRequest(request) && */
    /*             co_await mHttp->writeBodyStream(cachedStream) && */
    /*             co_await mHttp->sock.peekchar()) [[likely]] { */
    /*             co_return {}; */
    /*         } */
    /*         (void)cachedStream.seek(0); */
    /*         mHttp = nullptr; */
    /*     } */
    /*     co_return std::errc::connection_aborted; */
    /* } */
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
    BorrowedStream &extractSocket() const noexcept {
        return mHttp->sock;
    }

    Expected<> doConnect(std::string_view host,
                         std::chrono::steady_clock::duration timeout,
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
            return std::errc::protocol_not_supported;
        }
    }

    HTTPConnection() = default;

    Task<Expected<std::tuple<HTTPResponse, String>>>
    request(HTTPRequest req, std::string_view in = {}) {
        builtinHeaders(req);
        RAIIPointerResetter reset(&mHttp);
        co_await co_await tryWriteRequestAndBody(req, in);
        HTTPResponse res;
        String body;
        co_await co_await mHttp->readResponse(res);
        co_await co_await mHttp->readBody(body);
        reset.neverMind();
        co_return std::tuple{std::move(res), std::move(body)};
    }

    Task<Expected<std::tuple<HTTPResponse, OwningStream>>>
    request_streamed(HTTPRequest req, std::string_view in = {}) {
        builtinHeaders(req);
        RAIIPointerResetter reset(&mHttp);
        co_await co_await tryWriteRequestAndBody(req, in);
        HTTPResponse res;
        std::string body;
        co_await co_await mHttp->readResponse(res);
        auto [r, w] = pipe_stream();
        co_spawn(pipe_bind(std::move(w),
                           [this](OwningStream &w) -> Task<Expected<>> {
                               co_await co_await mHttp->readBodyStream(w);
                               co_return {};
                           }));
        reset.neverMind();
        co_return std::tuple{res, std::move(r)};
    }
};

struct HTTPConnectionPool {
private:
    struct alignas(hardware_destructive_interference_size) PoolEntry {
        HTTPConnection mHttp;
        std::atomic_bool mInuse{false};
        bool mValid{false};
        std::chrono::steady_clock::time_point mLastAccess;
    };

    struct HostPool {
        std::vector<PoolEntry> mPool;
        ConditionVariable mFreeSlot;

        explicit HostPool(std::size_t size) : mPool(size) {}
    };

    std::shared_mutex mMutex;
    SimpleMap<std::string, HostPool> mPools;
    std::chrono::steady_clock::duration mTimeout;
    std::chrono::steady_clock::duration mKeepAlive;
    std::chrono::steady_clock::time_point mLastGC;
    std::size_t mConnPerHost;
    bool mFollowProxy;

public:
    struct HTTPConnectionPtr {
    private:
        PoolEntry *mEntry;
        HostPool *mPool;

        explicit HTTPConnectionPtr(PoolEntry *entry, HostPool *pool) noexcept
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
                mEntry->mLastAccess = std::chrono::steady_clock::now();
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
        std::size_t connPerHost = 8,
        std::chrono::steady_clock::duration timeout = std::chrono::seconds(20),
        std::chrono::steady_clock::duration keepAlive = std::chrono::minutes(3),
        bool followProxy = true)
        : mTimeout(timeout),
          mKeepAlive(keepAlive),
          mConnPerHost(connPerHost),
          mFollowProxy(followProxy) {}

private:
    std::optional<Expected<HTTPConnectionPtr>>
    lookForFreeSlot(std::string_view host) /* MT-safe */ {
        std::shared_lock lock(mMutex);
        auto *pool = mPools.at(host);
        lock.unlock();
        if (!pool) {
            std::lock_guard wlock(mMutex);
            pool = mPools.at(host);
            if (!pool) [[likely]] {
                pool = &mPools.emplace(std::string(host), mConnPerHost);
            }
        }
        for (auto &entry: pool->mPool) {
            bool expected = false;
            if (entry.mInuse.compare_exchange_strong(
                    expected, true, std::memory_order_acq_rel)) {
                if (entry.mValid) {
                    entry.mLastAccess = std::chrono::steady_clock::now();
                } else {
                    if (auto e =
                            entry.mHttp.doConnect(host, mTimeout, mFollowProxy);
                        e.has_error()) [[unlikely]] {
                        return std::move(e).error();
                    }
                    entry.mLastAccess = std::chrono::steady_clock::now();
                    entry.mValid = true;
                }
                return HTTPConnectionPtr(&entry, pool);
            }
        }
        return std::nullopt;
    }

    Task<> waitForFreeSlot(std::string_view host) /* MT-safe */ {
        std::shared_lock lock(mMutex);
        auto *pool = mPools.at(host);
        lock.unlock();
        if (pool) [[likely]] {
            (void)co_await co_timeout(pool->mFreeSlot.wait(),
                                      std::chrono::milliseconds(100));
        }
        co_return;
    }

    void garbageCollect() /* MT-safe */ {
        auto now = std::chrono::steady_clock::now();
        if ((now - mLastGC) * 2 > mKeepAlive) {
            std::shared_lock lock(mMutex);
            for (auto &[_, pool]: mPools) {
                for (auto &entry: pool.mPool) {
                    bool expected = false;
                    if (entry.mInuse.compare_exchange_strong(
                            expected, true, std::memory_order_acq_rel)) {
                        if (entry.mValid &&
                            now - entry.mLastAccess > mKeepAlive) {
                            entry.mHttp.terminateLifetime();
                            entry.mValid = false;
                        }
                        entry.mInuse.store(false, std::memory_order_release);
                    }
                }
                mLastGC = now;
            }
        }
    }

public:
    Task<Expected<HTTPConnectionPtr>>
    connect(std::string_view host) /* MT-safe */ {
    again:
        garbageCollect();
        if (auto conn = lookForFreeSlot(host)) {
            co_return std::move(*conn);
        }
        co_await waitForFreeSlot(host);
        goto again;
    }
};
} // namespace co_async
