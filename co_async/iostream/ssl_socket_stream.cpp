#include <bearssl.h>
#include <co_async/awaiter/task.hpp>
#include <co_async/iostream/socket_stream.hpp>
#include <co_async/iostream/ssl_socket_stream.hpp>
#include <co_async/iostream/stream_base.hpp>
#include <co_async/net/socket_proxy.hpp>
#include <co_async/platform/error_handling.hpp>
#include <co_async/platform/fs.hpp>
#include <co_async/platform/platform_io.hpp>
#include <co_async/platform/socket.hpp>
#include <co_async/utils/pimpl.hpp>
#include <co_async/utils/string_utils.hpp>

namespace co_async {
std::error_category const &bearSSLCategory() {
    static struct : std::error_category {
        char const *name() const noexcept override {
            return "BearSSL";
        }

        std::string message(int e) const override {
            static std::pair<int, char const *> errors[] = {
                {
                    BR_ERR_OK,
                    "BR_ERR_OK",
                },
                {
                    BR_ERR_BAD_PARAM,
                    "BR_ERR_BAD_PARAM",
                },
                {
                    BR_ERR_BAD_STATE,
                    "BR_ERR_BAD_STATE",
                },
                {
                    BR_ERR_UNSUPPORTED_VERSION,
                    "BR_ERR_UNSUPPORTED_VERSION",
                },
                {
                    BR_ERR_BAD_VERSION,
                    "BR_ERR_BAD_VERSION",
                },
                {
                    BR_ERR_BAD_LENGTH,
                    "BR_ERR_BAD_LENGTH",
                },
                {
                    BR_ERR_TOO_LARGE,
                    "BR_ERR_TOO_LARGE",
                },
                {
                    BR_ERR_BAD_MAC,
                    "BR_ERR_BAD_MAC",
                },
                {
                    BR_ERR_NO_RANDOM,
                    "BR_ERR_NO_RANDOM",
                },
                {
                    BR_ERR_UNKNOWN_TYPE,
                    "BR_ERR_UNKNOWN_TYPE",
                },
                {
                    BR_ERR_UNEXPECTED,
                    "BR_ERR_UNEXPECTED",
                },
                {
                    BR_ERR_BAD_CCS,
                    "BR_ERR_BAD_CCS",
                },
                {
                    BR_ERR_BAD_ALERT,
                    "BR_ERR_BAD_ALERT",
                },
                {
                    BR_ERR_BAD_HANDSHAKE,
                    "BR_ERR_BAD_HANDSHAKE",
                },
                {
                    BR_ERR_OVERSIZED_ID,
                    "BR_ERR_OVERSIZED_ID",
                },
                {
                    BR_ERR_BAD_CIPHER_SUITE,
                    "BR_ERR_BAD_CIPHER_SUITE",
                },
                {
                    BR_ERR_BAD_COMPRESSION,
                    "BR_ERR_BAD_COMPRESSION",
                },
                {
                    BR_ERR_BAD_FRAGLEN,
                    "BR_ERR_BAD_FRAGLEN",
                },
                {
                    BR_ERR_BAD_SECRENEG,
                    "BR_ERR_BAD_SECRENEG",
                },
                {
                    BR_ERR_EXTRA_EXTENSION,
                    "BR_ERR_EXTRA_EXTENSION",
                },
                {
                    BR_ERR_BAD_SNI,
                    "BR_ERR_BAD_SNI",
                },
                {
                    BR_ERR_BAD_HELLO_DONE,
                    "BR_ERR_BAD_HELLO_DONE",
                },
                {
                    BR_ERR_LIMIT_EXCEEDED,
                    "BR_ERR_LIMIT_EXCEEDED",
                },
                {
                    BR_ERR_BAD_FINISHED,
                    "BR_ERR_BAD_FINISHED",
                },
                {
                    BR_ERR_RESUME_MISMATCH,
                    "BR_ERR_RESUME_MISMATCH",
                },
                {
                    BR_ERR_INVALID_ALGORITHM,
                    "BR_ERR_INVALID_ALGORITHM",
                },
                {
                    BR_ERR_BAD_SIGNATURE,
                    "BR_ERR_BAD_SIGNATURE",
                },
                {
                    BR_ERR_WRONG_KEY_USAGE,
                    "BR_ERR_WRONG_KEY_USAGE",
                },
                {
                    BR_ERR_NO_CLIENT_AUTH,
                    "BR_ERR_NO_CLIENT_AUTH",
                },
                {
                    BR_ERR_IO,
                    "BR_ERR_IO",
                },
                {
                    BR_ERR_X509_INVALID_VALUE,
                    "BR_ERR_X509_INVALID_VALUE",
                },
                {
                    BR_ERR_X509_TRUNCATED,
                    "BR_ERR_X509_TRUNCATED",
                },
                {
                    BR_ERR_X509_EMPTY_CHAIN,
                    "BR_ERR_X509_EMPTY_CHAIN",
                },
                {
                    BR_ERR_X509_INNER_TRUNC,
                    "BR_ERR_X509_INNER_TRUNC",
                },
                {
                    BR_ERR_X509_BAD_TAG_CLASS,
                    "BR_ERR_X509_BAD_TAG_CLASS",
                },
                {
                    BR_ERR_X509_BAD_TAG_VALUE,
                    "BR_ERR_X509_BAD_TAG_VALUE",
                },
                {
                    BR_ERR_X509_INDEFINITE_LENGTH,
                    "BR_ERR_X509_INDEFINITE_LENGTH",
                },
                {
                    BR_ERR_X509_EXTRA_ELEMENT,
                    "BR_ERR_X509_EXTRA_ELEMENT",
                },
                {
                    BR_ERR_X509_UNEXPECTED,
                    "BR_ERR_X509_UNEXPECTED",
                },
                {
                    BR_ERR_X509_NOT_CONSTRUCTED,
                    "BR_ERR_X509_NOT_CONSTRUCTED",
                },
                {
                    BR_ERR_X509_NOT_PRIMITIVE,
                    "BR_ERR_X509_NOT_PRIMITIVE",
                },
                {
                    BR_ERR_X509_PARTIAL_BYTE,
                    "BR_ERR_X509_PARTIAL_BYTE",
                },
                {
                    BR_ERR_X509_BAD_BOOLEAN,
                    "BR_ERR_X509_BAD_BOOLEAN",
                },
                {
                    BR_ERR_X509_OVERFLOW,
                    "BR_ERR_X509_OVERFLOW",
                },
                {
                    BR_ERR_X509_BAD_DN,
                    "BR_ERR_X509_BAD_DN",
                },
                {
                    BR_ERR_X509_BAD_TIME,
                    "BR_ERR_X509_BAD_TIME",
                },
                {
                    BR_ERR_X509_UNSUPPORTED,
                    "BR_ERR_X509_UNSUPPORTED",
                },
                {
                    BR_ERR_X509_LIMIT_EXCEEDED,
                    "BR_ERR_X509_LIMIT_EXCEEDED",
                },
                {
                    BR_ERR_X509_WRONG_KEY_TYPE,
                    "BR_ERR_X509_WRONG_KEY_TYPE",
                },
                {
                    BR_ERR_X509_BAD_SIGNATURE,
                    "BR_ERR_X509_BAD_SIGNATURE",
                },
                {
                    BR_ERR_X509_TIME_UNKNOWN,
                    "BR_ERR_X509_TIME_UNKNOWN",
                },
                {
                    BR_ERR_X509_EXPIRED,
                    "BR_ERR_X509_EXPIRED",
                },
                {
                    BR_ERR_X509_DN_MISMATCH,
                    "BR_ERR_X509_DN_MISMATCH",
                },
                {
                    BR_ERR_X509_BAD_SERVER_NAME,
                    "BR_ERR_X509_BAD_SERVER_NAME",
                },
                {
                    BR_ERR_X509_CRITICAL_EXTENSION,
                    "BR_ERR_X509_CRITICAL_EXTENSION",
                },
                {
                    BR_ERR_X509_NOT_CA,
                    "BR_ERR_X509_NOT_CA",
                },
                {
                    BR_ERR_X509_FORBIDDEN_KEY_USAGE,
                    "BR_ERR_X509_FORBIDDEN_KEY_USAGE",
                },
                {
                    BR_ERR_X509_WEAK_PUBLIC_KEY,
                    "BR_ERR_X509_WEAK_PUBLIC_KEY",
                },
                {
                    BR_ERR_X509_NOT_TRUSTED,
                    "BR_ERR_X509_NOT_TRUSTED",
                },
                {0, nullptr},
            };
            std::size_t u;
            for (u = 0; errors[u].second; u++) {
                if (errors[u].first == e) {
                    return errors[u].second;
                }
            }
            return std::to_string(e);
        }
    } instance;

    return instance;
}

namespace {
struct SSLPemDecoder {
private:
    std::unique_ptr<br_pem_decoder_context> pemDec =
        std::make_unique<br_pem_decoder_context>();
    std::string result;
    std::string objName;
    std::vector<std::pair<std::string, std::string>> objs;

    static void pemResultAppender(void *self, void const *buf,
                                  std::size_t len) {
        reinterpret_cast<SSLPemDecoder *>(self)->onResult(
            {reinterpret_cast<char const *>(buf), len});
    }

    void onResult(std::string_view s) {
        result.append(s);
    }

public:
    SSLPemDecoder() {
        br_pem_decoder_init(pemDec.get());
        br_pem_decoder_setdest(pemDec.get(), pemResultAppender, this);
    }

    Expected<> decode(std::string_view s) {
        while (auto n = br_pem_decoder_push(pemDec.get(), s.data(), s.size())) {
            switch (br_pem_decoder_event(pemDec.get())) {
            case BR_PEM_BEGIN_OBJ:
                objName = br_pem_decoder_name(pemDec.get());
                break;
            case BR_PEM_END_OBJ:
                objs.emplace_back(std::move(objName), std::move(result));
                result.clear();
                break;
            case BR_PEM_ERROR:
#if CO_ASYNC_DEBUG
                std::cerr << "PEM decoder error\n";
#endif
                return std::error_code(BR_ERR_X509_INVALID_VALUE, bearSSLCategory());
            }
            s.remove_prefix(n);
        }
        return {};
    }

    std::vector<std::pair<std::string, std::string>> const &objects() const {
        return objs;
    }

    static Expected<std::vector<std::string>> tryDecode(std::string_view s) {
        std::vector<std::string> res;
        if (s.find("-----BEGIN ") != s.npos) {
            SSLPemDecoder dec;
            if (auto e = dec.decode(s); !e) [[unlikely]] {
                return CO_ASYNC_ERROR_FORWARD(e);
            }
            for (auto &[k, v]: dec.objs) {
                res.push_back(std::move(v));
            }
        } else {
            res.emplace_back(s);
        }
        return res;
    }
};

struct SSLX509Decoder {
private:
    std::unique_ptr<br_x509_decoder_context> x509Dec =
        std::make_unique<br_x509_decoder_context>();
    std::string result;

    static void x509ResultAppender(void *self, void const *buf,
                                   std::size_t len) {
        reinterpret_cast<SSLX509Decoder *>(self)->onResult(
            {reinterpret_cast<char const *>(buf), len});
    }

    void onResult(std::string_view s) {
        result.append(s);
    }

public:
    SSLX509Decoder() {
        br_x509_decoder_init(x509Dec.get(), x509ResultAppender, this);
    }

    SSLX509Decoder &decode(std::string_view s) {
        br_x509_decoder_push(x509Dec.get(), s.data(), s.size());
        return *this;
    }

    Expected<std::string_view> getDN() const {
        int err = br_x509_decoder_last_error(x509Dec.get());
        if (err != BR_ERR_OK) [[unlikely]] {
#if CO_ASYNC_DEBUG
            std::cerr << "X509 decoder error: " +
                             bearSSLCategory().message(err) + "\n";
#endif
            return std::error_code(err, bearSSLCategory());
        }
        return result;
    }

    br_x509_pkey *getPubKey() const {
        return br_x509_decoder_get_pkey(x509Dec.get());
    }
};
} // namespace

struct SSLServerPrivateKey {
private:
    br_skey_decoder_context skeyDec;

public:
    SSLServerPrivateKey() {
        br_skey_decoder_init(&skeyDec);
    }

    SSLServerPrivateKey &decodeBinary(std::string_view s) {
        br_skey_decoder_push(&skeyDec, s.data(), s.size());
        return *this;
    }

    Expected<> set(std::string_view pkey) {
        if (auto e = SSLPemDecoder::tryDecode(pkey)) {
            for (auto &s: *e) {
                decodeBinary(s);
            }
            return {};
        } else {
            return CO_ASYNC_ERROR_FORWARD(e);
        }
    }

    br_ec_private_key const *getEC() const {
        return br_skey_decoder_get_ec(&skeyDec);
    }

    br_rsa_private_key const *getRSA() const {
        return br_skey_decoder_get_rsa(&skeyDec);
    }
};

struct SSLClientTrustAnchor {
public:
    std::vector<br_x509_trust_anchor> trustAnchors;

    Expected<> addBinary(std::string_view certX506) {
        auto &x506 = x506Stores.emplace_back();
        x506.decode(certX506);
        auto dn = x506.getDN();
        if (dn.has_error()) {
            return dn.error();
        }
        trustAnchors.push_back({
            {reinterpret_cast<unsigned char *>(const_cast<char *>(dn->data())),
             dn->size()},
            BR_X509_TA_CA,
            *x506.getPubKey(),
        });
        return {};
    }

    Expected<> add(std::string_view certX506) {
        if (auto e = SSLPemDecoder::tryDecode(certX506)) [[likely]] {
            for (auto &s: *e) {
                if (auto e = addBinary(s); !e) [[unlikely]] {
                    return CO_ASYNC_ERROR_FORWARD(e);
                }
            }
            return {};
        } else {
            return CO_ASYNC_ERROR_FORWARD(e);
        }
    }

    bool empty() const {
        return trustAnchors.empty();
    }

    void clear() {
        trustAnchors.clear();
    }

private:
    std::vector<SSLX509Decoder> x506Stores;
};

struct SSLServerCertificate {
public:
    std::vector<br_x509_certificate> certificates;

    void addBinary(std::string certX506) {
        auto &cert = strStores.emplace_back(std::move(certX506));
        certificates.push_back(
            {reinterpret_cast<unsigned char *>(cert.data()), cert.size()});
    }

    Expected<> add(std::string_view certX506) {
        if (auto e = SSLPemDecoder::tryDecode(certX506)) [[likely]] {
            for (auto &s: *e) {
                addBinary(s);
            }
            return {};
        } else {
            return CO_ASYNC_ERROR_FORWARD(e);
        }
    }

private:
    std::vector<String> strStores;
};

struct SSLServerSessionCache {
    std::unique_ptr<unsigned char[]> mLruBuf;
    br_ssl_session_cache_lru mLru;

    explicit SSLServerSessionCache(std::size_t size = 512) {
        mLruBuf = std::make_unique<unsigned char[]>(size);
        br_ssl_session_cache_lru_init(&mLru, mLruBuf.get(), size);
    }
};

namespace {
struct SSLSocketStream : Stream {
private:
    SocketStream raw;
    br_ssl_engine_context *eng = nullptr;
    std::unique_ptr<char[]> iobuf =
        std::make_unique<char[]>(BR_SSL_BUFSIZE_BIDI);

public:
    explicit SSLSocketStream(SocketHandle file) : raw(std::move(file)) {}

protected:
    void setEngine(br_ssl_engine_context *eng_) {
        eng = eng_;
        br_ssl_engine_set_buffer(eng, iobuf.get(), BR_SSL_BUFSIZE_BIDI, 1);
    }

    Task<Expected<>> bearSSLRunUntil(unsigned target) {
        for (;;) {
            unsigned state = br_ssl_engine_current_state(eng);
            if (state & BR_SSL_CLOSED) {
                int err = br_ssl_engine_last_error(eng);
                if (err != BR_ERR_OK) [[unlikely]] {
#if CO_ASYNC_DEBUG
                    std::cerr
                        << "SSL error: " + bearSSLCategory().message(err) +
                               "\n";
#endif
                    co_return std::error_code(err, bearSSLCategory());
                }
                co_return {};
            }
            if (state & BR_SSL_SENDREC) {
                unsigned char *buf;
                std::size_t len, wlen;
                buf = br_ssl_engine_sendrec_buf(eng, &len);
                if (auto e = co_await raw.raw_write(
                        {reinterpret_cast<char const *>(buf), len});
                    e && *e != 0) {
                    wlen = *e;
                } else {
                    if (!eng->shutdown_recv) [[unlikely]] {
                        if (eng->iomode != 0) {
                            eng->iomode = 0;
                            eng->err = BR_ERR_IO;
                        }
                        if (e.has_error()) {
                            co_return CO_ASYNC_ERROR_FORWARD(e);
                        } else {
                            co_return std::errc::broken_pipe;
                        }
                    } else if (e.has_error()) [[unlikely]] {
                        co_return CO_ASYNC_ERROR_FORWARD(e);
                    } else {
                        co_return {};
                    }
                }
                br_ssl_engine_sendrec_ack(eng, wlen);
                continue;
            }
            if (state & target) {
                co_return {};
            }
            if (state & BR_SSL_RECVAPP) [[unlikely]] {
#if CO_ASYNC_DEBUG
                std::cerr << "SSL write not ready\n";
#endif
                co_return std::errc::broken_pipe;
                // throw std::runtime_error("SSL write not ready");
            }
            if (state & BR_SSL_RECVREC) {
                unsigned char *buf;
                std::size_t len, rlen;
                buf = br_ssl_engine_recvrec_buf(eng, &len);
                if (auto e = co_await raw.raw_read(
                        {reinterpret_cast<char *>(buf), len});
                    e && *e != 0) {
                    rlen = *e;
                } else {
                    if (!eng->shutdown_recv) [[unlikely]] {
                        if (eng->iomode != 0) {
                            eng->iomode = 0;
                            eng->err = BR_ERR_IO;
                        }
                        if (e.has_error()) {
                            co_return CO_ASYNC_ERROR_FORWARD(e);
                        } else {
                            co_return std::errc::broken_pipe;
                        }
                    } else if (e.has_error()) [[unlikely]] {
                        co_return CO_ASYNC_ERROR_FORWARD(e);
                    } else {
                        co_return {};
                    }
                }
                br_ssl_engine_recvrec_ack(eng, rlen);
                continue;
            }
        }
    }

public:
    Task<Expected<>> raw_flush() override {
        br_ssl_engine_flush(eng, 0);
        co_await co_await bearSSLRunUntil(BR_SSL_SENDAPP | BR_SSL_RECVAPP);
        co_return {};
    }

    Task<Expected<std::size_t>> raw_read(std::span<char> buffer) override {
        unsigned char *buf;
        std::size_t alen;
        if (buffer.empty()) [[unlikely]] {
            co_return std::size_t(0);
        }
        co_await co_await bearSSLRunUntil(BR_SSL_RECVAPP);
        buf = br_ssl_engine_recvapp_buf(eng, &alen);
        if (alen > buffer.size()) {
            alen = buffer.size();
        }
        memcpy(buffer.data(), buf, alen);
        br_ssl_engine_recvapp_ack(eng, alen);
        co_return alen;
    }

    Task<Expected<std::size_t>>
    raw_write(std::span<char const> buffer) override {
        unsigned char *buf;
        std::size_t alen;
        if (buffer.empty()) [[unlikely]] {
            co_return std::size_t(0);
        }
        co_await co_await bearSSLRunUntil(BR_SSL_SENDAPP);
        buf = br_ssl_engine_sendapp_buf(eng, &alen);
        if (alen > buffer.size()) {
            alen = buffer.size();
        }
        memcpy(buf, buffer.data(), alen);
        br_ssl_engine_sendapp_ack(eng, alen);
        co_return alen;
    }

    Task<> raw_close() override {
#if CO_ASYNC_DEBUG
        if (br_ssl_engine_current_state(eng) != BR_SSL_CLOSED) [[unlikely]] {
            std::cerr << "SSL closed improperly\n"
                      << br_ssl_engine_current_state(eng);
        }
#endif
        br_ssl_engine_close(eng);
        eng = nullptr;
        co_return;
    }

    void raw_timeout(std::chrono::steady_clock::duration timeout) override {
        raw.raw_timeout(timeout);
    }
};

struct SSLServerSocketStream : SSLSocketStream {
private:
    std::unique_ptr<br_ssl_server_context> ctx =
        std::make_unique<br_ssl_server_context>();

public:
    explicit SSLServerSocketStream(SocketHandle file,
                                   SSLServerCertificate const &cert,
                                   SSLServerPrivateKey const &pkey,
                                   std::span<char const *const> protocols,
                                   SSLServerSessionCache *cache = nullptr)
        : SSLSocketStream(std::move(file)) {
        if (auto rsa = pkey.getRSA()) {
            br_ssl_server_init_full_rsa(ctx.get(), std::data(cert.certificates),
                                        std::size(cert.certificates), rsa);
        } else if (auto ec = pkey.getEC()) {
            br_ssl_server_init_full_ec(ctx.get(), std::data(cert.certificates),
                                       std::size(cert.certificates),
                                       BR_KEYTYPE_EC, ec);
        } else [[unlikely]] {
            throw std::runtime_error(
                "invalid private key type, must be either RSA or EC");
        }
        setEngine(&ctx->eng);
        if (cache) {
            br_ssl_server_set_cache(ctx.get(), &cache->mLru.vtable);
        }
        br_ssl_engine_set_protocol_names(
            &ctx->eng, const_cast<char const **>(protocols.data()),
            protocols.size());
        br_ssl_server_reset(ctx.get());
    }
};

struct SSLClientSocketStream : SSLSocketStream {
private:
    std::unique_ptr<br_ssl_client_context> ctx =
        std::make_unique<br_ssl_client_context>();
    std::unique_ptr<br_x509_minimal_context> x509Ctx =
        std::make_unique<br_x509_minimal_context>();

public:
    explicit SSLClientSocketStream(SocketHandle file,
                                   SSLClientTrustAnchor const &ta,
                                   char const *host,
                                   std::span<char const *const> protocols)
        : SSLSocketStream(std::move(file)) {
        br_ssl_client_init_full(ctx.get(), x509Ctx.get(),
                                std::data(ta.trustAnchors),
                                std::size(ta.trustAnchors));
        setEngine(&ctx->eng);
        br_ssl_engine_set_protocol_names(
            &ctx->eng, const_cast<char const **>(protocols.data()),
            protocols.size());
        br_ssl_client_reset(ctx.get(), host, 0);
    }

    void ssl_reset(char const *host, bool resume) {
        br_ssl_client_reset(ctx.get(), host, resume);
    }

    std::string ssl_get_selected_protocol() {
        if (auto p = br_ssl_engine_get_selected_protocol(&ctx->eng)) {
            return p;
        }
        return {};
    }
};
} // namespace

Task<Expected<OwningStream>>
ssl_connect(char const *host, int port, SSLClientTrustAnchor const &ta,
            std::span<char const *const> protocols, std::string_view proxy,
            std::chrono::steady_clock::duration timeout) {
    auto conn =
        co_await co_await socket_proxy_connect(host, port, proxy, timeout);
    auto sock = make_stream<SSLClientSocketStream>(std::move(conn), ta, host,
                                                   protocols);
    sock.timeout(timeout);
    co_return sock;
}

OwningStream ssl_accept(SocketHandle file, SSLServerCertificate const &cert,
                        SSLServerPrivateKey const &pkey,
                        std::span<char const *const> protocols,
                        SSLServerSessionCache *cache) {
    return make_stream<SSLServerSocketStream>(std::move(file), cert, pkey,
                                              protocols, cache);
}

DefinePImpl(SSLServerPrivateKey);
DefinePImpl(SSLClientTrustAnchor);
Expected<> ForwardPImplMethod(SSLClientTrustAnchor, add,
                              (std::string_view content), content);
DefinePImpl(SSLServerCertificate);
Expected<> ForwardPImplMethod(SSLServerCertificate, add, (std::string_view content),
                        content);
DefinePImpl(SSLServerSessionCache);
} // namespace co_async
