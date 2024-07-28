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
    std::optional<double> frequency_penalty{};
    std::optional<int> max_tokens{};
    std::optional<double> persence_penalty{};
    std::optional<std::vector<std::string>> stop{};
    std::optional<double> temperature{};
    std::optional<double> top_p{};
    std::optional<bool> logprobs{};
    std::optional<int> top_logprobs{};
    std::optional<bool> stream{};

    REFLECT(messages, model, frequency_penalty, max_tokens, persence_penalty, stop, temperature, top_p, logprobs, top_logprobs, stream);
};

struct ChatCompletionResult {
    struct Choice {
        struct Message {
            std::optional<std::string> role;
            std::optional<std::string> content;

            REFLECT(role, content);
        };

        struct Logprobs {
            struct Content {
                struct Logprob {
                    std::string token;
                    double logprob;
                    std::vector<std::uint8_t> bytes;

                    REFLECT(token, logprob, bytes);
                };

                std::string token;
                double logprob;
                std::vector<std::uint8_t> bytes;
                std::vector<Logprob> top_logprobs;

                REFLECT(token, logprob, bytes, top_logprobs);
            };

            std::vector<Content> content;

            REFLECT(content);
        };

        int index;
        std::optional<Message> delta;
        std::optional<Message> message;
        std::optional<std::string> finish_reason;
        std::optional<Logprobs> logprobs;

        REFLECT(index, delta, finish_reason, logprobs);
    };

    struct Usage {
        int completion_tokens;
        int prompt_tokens;
        int total_tokens;

        REFLECT(completion_tokens, prompt_tokens, total_tokens);
    };

    std::string id;
    std::vector<Choice> choices;
    std::int64_t created;
    std::string model;
    std::optional<Usage> usage;

    REFLECT(id, choices, created, model, usage);
};

static Task<Expected<>> amain() {
    std::string authorization;
    if (auto p = std::getenv("DEEPSEEK_API_KEY")) {
        authorization.append("Bearer ");
        authorization.append(p);
    }
    HTTPConnectionPool pool;
    for (std::size_t i = 0; i < 1; ++i) {
        auto conn = co_await co_await pool.connect("https://api.deepseek.com");
        HTTPRequest req = {
            .method = "POST",
            .uri = URI::parse("/chat/completions"),
            .headers = {
                {"content-type", "application/json"},
                {"authorization", String{authorization}},
            },
        };
        auto compReq = ChatCompletionRequest{
            .messages = {
                /* {.role = "user", .content = "Why Google hates C++ exceptions?"}, */
                {.role = "user", .content = "What is std::error_code?"},
                /* {.role = "user", .content = "What is the difference between std::error_code and std::error_condition?"}, */
            },
            .model = "deepseek-coder",
            .stream = true,
        };
        auto [res, body] = co_await co_await conn->request_streamed(req, json_encode(compReq));
        while (auto tmp = co_await body.getline('\n')) {
            std::string_view line = *tmp;
            if (line.starts_with("data: "sv)) {
                line.remove_prefix(6);
                if (line == "[DONE]"sv) {
                    break;
                }
                /* std::cerr << line << '\n'; */
                auto compRes = co_await json_decode<ChatCompletionResult>(line);
                /* debug(), compRes; */
                co_await co_await stdio().puts(compRes.choices.at(0).delta.value().content.value_or(""));
                co_await co_await stdio().flush();
            }
        }
        co_await co_await body.dropall();
        co_await co_await stdio().putchar('\n');
        co_await co_await stdio().flush();
    }
    co_return {};
}

int main() {
    /* auto tmp = reflect::json_decode<ChatCompletionStreamingResult>(R"json({"id":"1234","choices":[{"index":1,"delta":{"role":"user","content":"Hello?"},"finish_reason":null}],"model":"deepseek-coder"})json"); */
    /* debug(), tmp; */
    co_main(amain());
    return 0;
}
