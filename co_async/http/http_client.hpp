#pragma once /*{export module co_async:http.http_client;}*/

#include <co_async/std.hpp>                  /*{import std;}*/
#include <co_async/awaiter/task.hpp>         /*{import :awaiter.task;}*/
#include <co_async/http/http11.hpp>          /*{import :http.http11;}*/
#include <co_async/http/http_status_code.hpp>/*{import :http.http_status_code;}*/
#include <co_async/utils/string_utils.hpp>   /*{import :utils.string_utils;}*/
#include <co_async/utils/simple_map.hpp>     /*{import :utils.simple_map;}*/
#include <co_async/system/socket.hpp>        /*{import :system.socket;}*/
#include <co_async/system/fs.hpp>            /*{import :system.fs;}*/
#include <co_async/http/uri.hpp>             /*{import :http.uri;}*/
#include <co_async/http/http11.hpp>          /*{import :http.http11;}*/
#include <co_async/iostream/socket_stream.hpp>/*{import :iostream.socket_stream;}*/
#include <co_async/iostream/ssl_socket_stream.hpp>/*{import :iostream.ssl_socket_stream;}*/

namespace co_async {

template <class K, class V>
struct LruMap {
};

struct HTTPConnectionBase {
    HTTPConnectionBase() = default;
    HTTPConnectionBase(HTTPConnectionBase &&) = delete;

    virtual ~HTTPConnectionBase() = default;

protected:
    virtual Task<> doConnect(std::string_view host, int port) = 0;
    virtual Task<> doWriteHeader() = 0;
    virtual Task<> doWriteBody(std::string_view body) = 0;
    virtual Task<> doWriteBodyStream(FileStream &body) = 0;
    virtual Task<> doWriteNoBody() = 0;
    virtual Task<bool> doReadHeader() = 0;
    virtual Task<std::string> doReadBody() = 0;
    virtual Task<> doReadBodyStream(FileStream &body) = 0;
    virtual Task<> doSendProxyConnect(std::string host) = 0;

    std::unique_ptr<HTTPRequest> mReq;
    std::unique_ptr<HTTPResponse> mRes;
    std::string mHostName;
#if CO_ASYNC_DEBUG
    int mPhase = -1;
#endif

public:
    std::atomic_bool mOwned{false};

    Task<> connectTo(std::string_view hostName, int defaultPort, std::string_view proxy) {
        int port = defaultPort;
        auto host = hostName;
        if (auto i = host.rfind(':'); i != host.npos) {
            if (auto portOpt = from_string<int>(host.substr(i + 1))) {
                port = *portOpt;
                host.remove_suffix(host.size() - i);
            }
        }
        if (proxy.empty()) {
            co_await doConnect(host, port);
        } else {
            auto proxyHost = proxy;
            auto proxyPort = 80;
            if (auto i = proxyHost.rfind(':'); i != proxyHost.npos) {
                if (auto portOpt = from_string<int>(proxyHost.substr(i + 1))) {
                    proxyPort = *portOpt;
                    proxyHost.remove_suffix(proxyHost.size() - i);
                }
            }
            co_await doConnect(proxyHost, proxyPort);
            co_await doSendProxyConnect(std::string(host) + ":" + to_string(port));
        }
        mReq = std::make_unique<HTTPRequest>();
        mRes = std::make_unique<HTTPResponse>();
        mHostName = hostName;
        mPhase = 0;
    }

    Task<> write_header(HTTPRequest const &req) {
#if CO_ASYNC_DEBUG
        if (mPhase != 0) [[unlikely]] {
            throw std::runtime_error("write_header must be the first");
        }
        mPhase = 1;
#endif
        using namespace std::string_literals;
        *mReq = req;
        mReq->headers.insert("user-agent"s, "co_async/0.0.1"s);
        mReq->headers.insert("host"s, mHostName);
        /* mReq->headers.insert("accept-encoding"s, "gzip, deflate, br, compress"s); */
        /* mReq->headers.insert("accept-encoding"s, "deflate"s); */
        mReq->headers.insert("accept"s, "*/*"s);
        co_await doWriteHeader();
    }

    Task<> write_body(std::string_view body) {
#if CO_ASYNC_DEBUG
        if (mPhase != 1) [[unlikely]] {
            throw std::runtime_error("write_body must be called after write_header");
        }
        mPhase = 2;
#endif
        co_await doWriteBody(body);
    }

    Task<> write_body_stream(FileStream &body) {
#if CO_ASYNC_DEBUG
        if (mPhase != 1) [[unlikely]] {
            throw std::runtime_error("write_body_stream must be called after write_header");
        }
        mPhase = 2;
#endif
        co_await doWriteBodyStream(body);
    }

    Task<> write_nobody() {
#if CO_ASYNC_DEBUG
        if (mPhase != 1) [[unlikely]] {
            throw std::runtime_error("write_nobody must be called after write_header");
        }
        mPhase = 2;
#endif
        co_await doWriteNoBody();
    }

    Task<HTTPResponse> read_header() {
#if CO_ASYNC_DEBUG
        if (mPhase != 2) [[unlikely]] {
            throw std::runtime_error("read_header must be called after write_body");
        }
        mPhase = 3;
#endif
        if (!co_await doReadHeader()) [[unlikely]] {
            throw std::runtime_error("failed to read header, server closed?");
        }
        co_return *mRes;
    }

    Task<std::string> read_body() {
#if CO_ASYNC_DEBUG
        if (mPhase != 3) [[unlikely]] {
            throw std::runtime_error("read_body must be called after read_header");
        }
        mPhase = 0;
#endif
        co_return co_await doReadBody();
    }

    Task<> read_body_stream(FileStream &body) {
#if CO_ASYNC_DEBUG
        if (mPhase != 3) [[unlikely]] {
            throw std::runtime_error("read_body_stream must be called after read_header");
        }
        mPhase = 0;
#endif
        co_await doReadBodyStream(body);
    }
};

template <class HTTP>
struct HTTPConnectionImpl : HTTPConnectionBase {
protected:
    std::optional<HTTP> mHttp;

public:
    Task<> doWriteHeader() override {
        co_await mHttp->write_header(*mReq);
    }

    Task<> doWriteBody(std::string_view body) override {
        co_await mHttp->write_body(*mReq, body);
    }

    Task<> doWriteBodyStream(FileStream &body) override {
        co_await mHttp->write_body_stream(*mReq, body);
    }

    Task<> doWriteNoBody() override {
        co_await mHttp->write_nobody(*mReq);
    }

    Task<bool> doReadHeader() override {
        co_return co_await mHttp->read_header(*mRes);
    }

    Task<std::string> doReadBody() override {
        co_return co_await mHttp->read_body(*mRes);
    }

    Task<> doReadBodyStream(FileStream &body) override {
        co_await mHttp->read_body_stream(*mRes, body);
    }

    Task<> doSendProxyConnect(std::string host) override {
        co_await mHttp->sock.puts("CONNECT " + host + " HTTP/1.1\r\nHost: " + host + "\r\nProxy-Connection: Keep-Alive\r\n\r\n");
    }
};

struct HTTPConnectionHTTP : HTTPConnectionImpl<HTTP11<SocketStream>> {
    Task<> doConnect(std::string_view host, int port) override {
        mHttp.emplace(co_await SocketStream::connect(std::string(host).c_str(), port));
    }
};

struct HTTPConnectionHTTPS : HTTPConnectionImpl<HTTP11<SSLClientSocketStream>> {
protected:
    inline static SSLClientTrustAnchor mTrustAnchors;

public:
    static Task<> loadCACertificates() {
        auto path = make_path("/etc/ssl/certs/ca-certificates.crt");
        if (co_await fs_stat(path)) {
            mTrustAnchors.add(co_await file_read(path));
        }
    }

    Task<> doConnect(std::string_view host, int port) override {
        mHttp.emplace(co_await SSLClientSocketStream::connect(std::string(host).c_str(), port, mTrustAnchors));
    }
};

/*[export]*/ struct HTTPClientPool {
    struct Connection {
        HTTPConnectionBase *mConn;

        explicit Connection(HTTPConnectionBase *conn) noexcept : mConn(conn) {
        }

        Connection(Connection &&that) noexcept : mConn(std::exchange(that.mConn, nullptr)) {}

        Connection &operator=(Connection &&that) noexcept {
            std::swap(mConn, that.mConn);
            return *this;
        }

        HTTPConnectionBase &operator*() const noexcept {
            return *mConn;
        }

        HTTPConnectionBase *operator->() const noexcept {
            return mConn;
        }

        ~Connection() {
            if (mConn) {
                mConn->mOwned.store(false, std::memory_order_release);
            }
        }
    };

    Task<std::unique_ptr<HTTPConnectionBase>> createConnection(std::string_view host) {
        std::unique_ptr<HTTPConnectionBase> conn;
        if (host.starts_with("https://")) {
            host.remove_prefix(8);
            conn = std::make_unique<HTTPConnectionHTTPS>();
            co_await conn->connectTo(host, 443, https_proxy);
        } else if (host.starts_with("http://")) {
            host.remove_prefix(7);
            conn = std::make_unique<HTTPConnectionHTTP>();
            co_await conn->connectTo(host, 80, http_proxy);
        } else {
            throw std::runtime_error("invalid protocol, must be http:// or https://");
        }
        co_return conn;
    }

    Task<Connection> connect(std::string_view host) {
        std::unique_lock lck(mMutex);
        if (auto ppConn = mHttpPool.at(host)) {
            for (auto const &pConn: *ppConn) {
                bool expect = false;
                if (pConn->mOwned.compare_exchange_strong(expect, true, std::memory_order_acq_rel)) {
                    co_return Connection(pConn.get());
                }
            }
        }
        lck.unlock();
        auto conn = co_await createConnection(host);
        conn->mOwned.store(true, std::memory_order_relaxed);
        auto connPtr = conn.get();
        lck.lock();
        mHttpPool.insert(std::string(host), {}).push_back(std::move(conn));
        co_return Connection(connPtr);
    }

    static Task<> load_ca_certifi() {
        co_await HTTPConnectionHTTPS::loadCACertificates();
    }

    void follow_http_proxy() {
        if (auto p = std::getenv("http_proxy")) {
            http_proxy = p;
        }
        if (auto p = std::getenv("https_proxy")) {
            https_proxy = p;
        }
    }

private:
    std::mutex mMutex; // todo: AsyncMutex instead?
    SimpleMap<std::string, std::vector<std::unique_ptr<HTTPConnectionBase>>> mHttpPool;
    std::string http_proxy;
    std::string https_proxy;
};

}
