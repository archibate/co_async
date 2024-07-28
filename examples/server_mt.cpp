#include <co_async/co_async.hpp>
#include <co_async/std.hpp>

using namespace co_async;
using namespace std::literals;

static Task<Expected<>> amain(std::string serveAt) {
    co_await co_await stdio().putline("listening at: "s + serveAt);
    auto listener = co_await co_await listener_bind(co_await AddressResolver().host(serveAt).resolve_one());

    HTTPServer server;
    server.route("GET", "/", [](HTTPServer::IO &io) -> Task<Expected<>> {
        co_await co_await HTTPServerUtils::make_ok_response(io, "<h1>It works!</h1>");
        co_return {};
    });

    struct Worker {
        std::deque<Task<Expected<>>> q;
        std::condition_variable cv;
        std::mutex mtx;
        std::jthread th;

        void spawn(Task<Expected<>> task) {
            std::lock_guard lck(mtx);
            q.push_back(std::move(task));
            cv.notify_all();
        }

        void start(std::size_t i) {
            th = std::jthread([this, i] (std::stop_token stop) {
                IOContext ctx;
                PlatformIOContext::schedSetThreadAffinity(i);
                while (!stop.stop_requested()) [[likely]] {
                    while (ctx.runOnce()) {
                        std::lock_guard lck(mtx);
                        if (!q.empty())
                            break;
                    }
                    std::unique_lock lck(mtx);
                    cv.wait(lck, [this] { return !q.empty(); });
                    auto task = std::move(q.front());
                    q.pop_front();
                    lck.unlock();
                    co_spawn(std::move(task));
                }
            });
        }
    };
    std::vector<Worker> workers(std::thread::hardware_concurrency());

    for (std::size_t i = 0; i < workers.size(); ++i) {
        workers[i].start(i);
    }

    std::size_t i = 0;
    while (true) {
        if (auto income = co_await listener_accept(listener)) [[likely]] {
            workers[i].spawn(server.handle_http(std::move(*income)));
            ++i;
            if (i >= workers.size()) {
                i = 0;
            }
        }
    }
    co_return {};
}

int main(int argc, char **argv) {
    std::string serveAt = "127.0.0.1:8080";
    if (argc > 1) {
        serveAt = argv[1];
    }
    co_main(amain(serveAt));
    return 0;
}
