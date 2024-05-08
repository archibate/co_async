#include <co_async/co_async.hpp>
#include <co_async/std.hpp>

using namespace co_async;
using namespace std::literals;

struct ChatCompletionRequest {
    struct Message {
        std::string role;
        std::string content;

        REFLECT(role, content);
    };

    std::vector<Message> messages;
    std::string model;
    std::optional<double> frequency_penalty;
    std::optional<int> max_tokens;
    std::optional<double> persence_penalty;
    std::optional<std::vector<std::string>> stop;
    std::optional<double> temperature;
    std::optional<double> top_p;
    std::optional<bool> logprobs;
    std::optional<int> top_logprobs;
    std::optional<bool> stream;

    REFLECT(messages, model, frequency_penalty, max_tokens, persence_penalty, stop, temperature, top_p, logprobs, top_logprobs, stream);
};

struct ChatCompletionStreamingResult {
    struct Choice {
        struct Delta {
            std::optional<std::string> role;
            std::optional<std::string> content;

            REFLECT(role, content);
        };

        int index;
        Delta delta;
        std::optional<std::string> finish_reason;

        REFLECT(index, delta, finish_reason);
    };

    std::string id;
    std::vector<Choice> choices;
    std::int64_t created;
    std::string model;

    REFLECT(id, choices, created, model);
};

Task<Expected<void, std::errc>> amain() {
    std::string authorization;
    if (auto p = std::getenv("DEEPSEEK_API_KEY")) {
        authorization.append("Bearer ");
        authorization.append(p);
    }
    co_await https_load_ca_certificates();
    HTTPConnectionPool pool;
    for (std::size_t i = 0; i < 1; ++i) {
        auto conn = co_await co_await pool.connect("https://api.deepseek.com");
        HTTPRequest req = {
            .method = "POST",
            .uri = URI::parse("/chat/completions"),
            .headers = {
                {"content-type", "application/json"},
                {"authorization", authorization},
            },
        };
        auto compReq = ChatCompletionRequest{
            .messages = {
                {.role = "user", .content = "Why Google hates C++ exceptions?"},
            },
            .model = "deepseek-coder",
            .stream = true,
        };
        auto [res, body] = co_await co_await conn->requestStreamed(req, reflect::json_encode(compReq));
        while (auto tmp = co_await body.getline('\n')) {
            std::string_view line = *tmp;
            if (line.starts_with("data: "sv)) {
                line.remove_prefix(6);
                if (line == "[DONE]"sv) {
                    break;
                }
                /* std::cerr << line << '\n'; */
                auto compRes = reflect::json_decode<ChatCompletionStreamingResult>(line);
                /* debug(), compRes; */
                co_await co_await stdio().puts(compRes.choices.at(0).delta.content.value_or(""));
                co_await co_await stdio().flush();
            }
        }
        co_await body.dropall();
        co_await co_await stdio().putchar('\n');
        co_await co_await stdio().flush();
    }
    co_return {};
}

int main() {
    /* auto tmp = reflect::json_decode<ChatCompletionStreamingResult>(R"json({"id":"1234","choices":[{"index":1,"delta":{"role":"user","content":"Hello?"},"finish_reason":null}],"model":"deepseek-coder"})json"); */
    /* debug(), tmp; */
    co_synchronize(amain()).value();
    return 0;
}
