#include <co_async/std.hpp>
#include <co_async/co_async.hpp>
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#include <rime_api.h>

using namespace co_async;
using namespace std::literals;

/* ThreadPool pool; */

struct RPC_get_candidates {
    std::string key_sequence;
    std::size_t max_candidates;
    std::string user_data_dir;
    std::string shared_data_dir;

    REFLECT(key_sequence, max_candidates, user_data_dir, shared_data_dir);

    struct Result {
        struct Candidate {
            std::string text;
            std::string comment;

            REFLECT(text, comment);
        };

        std::vector<Candidate> candidates;

        REFLECT(candidates);
    };

    Task<Expected<Result>> operator()() {
        if (!max_candidates || key_sequence.empty()) {
            co_return Result{};
        }

        static CallOnce once;
        if (auto locked = co_await once.call_once()) {
            RIME_STRUCT(RimeTraits, rime);
            rime.distribution_name = "Rime";
            rime.distribution_code_name = "httprime";
            rime.distribution_version = "httprime";
            rime.app_name = "rime.httprime";

            rime.shared_data_dir = shared_data_dir.c_str();
            rime.user_data_dir = user_data_dir.c_str();
            rime.log_dir = "/tmp/";

            RimeSetup(&rime);
            RimeInitialize(&rime);
            RimeStartMaintenance(false);

            locked.set_ready();
        }

        RimeSessionId session_id = RimeCreateSession();
        while (!session_id) {
            co_await co_await co_sleep(10ms);
            session_id = RimeCreateSession();
        }

        if (!session_id) {
            co_return Unexpected{std::make_error_code(std::errc::io_error)};
        }

        /* co_return co_await co_await pool.run([this, session_id]() ->
         * Expected<Result> { */
        if (!RimeSimulateKeySequence(session_id, key_sequence.c_str())) {
            RimeDestroySession(session_id);
            co_return Unexpected{std::make_error_code(std::errc::io_error)};
        }

        RIME_STRUCT(RimeContext, context);
        if (!RimeGetContext(session_id, &context)) {
            RimeDestroySession(session_id);
            co_return Unexpected{std::make_error_code(std::errc::io_error)};
        }

        if (!context.menu.candidates) {
            RimeFreeContext(&context);
            RimeDestroySession(session_id);
            co_return Result{};
        }

        std::vector<Result::Candidate> result;
        while (true) {
            for (int i = 0; i < context.menu.num_candidates; i++) {
                RimeCandidate candidate = context.menu.candidates[i];
                result.push_back({candidate.text,
                                  candidate.comment ? candidate.comment : ""});
                if (result.size() >= max_candidates) {
                    RimeFreeContext(&context);
                    RimeDestroySession(session_id);
                    goto out;
                }
            }

            if (context.menu.is_last_page) {
                RimeFreeContext(&context);
                RimeDestroySession(session_id);
                goto out;
            }

            RimeProcessKey(session_id, (int)'=', 0);
            if (!RimeGetContext(session_id, &context)) {
                goto out;
            }
        }
    out:
        co_return Result{.candidates = std::move(result)};
        /* }); */
    }
};

static Task<Expected<>> amain(std::string addr) {
    co_await co_await stdio().putline("listening at: "s + addr);
    auto listener =
        co_await co_await listener_bind(co_await SocketAddress::parse(addr));
    HTTPServer server;
    server.route("POST", "/rpc/get_candidates",
                 [](HTTPServer::IO &io) -> Task<Expected<>> {
                     auto body = json_encode(co_await co_await (
                         co_await json_decode<RPC_get_candidates>(
                             co_await co_await io.request_body()))());
                     co_await co_await HTTPServerUtils::make_ok_response(
                         io, body, "application/json");
                     co_return {};
                 });

    while (true) {
        if (auto income = co_await listener_accept(listener)) {
            co_spawn(server.handle_http(std::move(*income)));
        }
    }
}

int main(int argc, char **argv) {
    std::string addr = argv[1] ? argv[1] : "127.0.0.1:8080"s;
    IOContext().join(amain(addr)).value();
    return 0;
}
