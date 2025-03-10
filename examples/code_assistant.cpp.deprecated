// #include <co_async/co_async.hpp>
// #include <co_async/std.hpp>
//
// using namespace co_async;
// using namespace std::literals;
//
// struct ChatCompletionRequest {
//     struct Message {
//         std::string role;
//         std::string content;
//
//         REFLECT(role, content);
//     };
//
//     std::vector<Message> messages;
//     std::string model;
//     std::optional<double> frequency_penalty{};
//     std::optional<int> max_tokens{};
//     std::optional<double> presence_penalty{};
//     std::optional<std::vector<std::string>> stop{};
//     std::optional<double> temperature{};
//     std::optional<double> top_p{};
//     std::optional<bool> logprobs{};
//     std::optional<int> top_logprobs{};
//     std::optional<bool> stream{};
//
//     REFLECT(messages, model, frequency_penalty, max_tokens, presence_penalty, stop, temperature, top_p, logprobs, top_logprobs, stream);
// };
//
// struct ChatCompletionResult {
//     struct Choice {
//         struct Message {
//             std::optional<std::string> role;
//             std::optional<std::string> content;
//
//             REFLECT(role, content);
//         };
//
//         struct Logprobs {
//             struct Content {
//                 struct Logprob {
//                     std::string token;
//                     double logprob;
//                     std::vector<std::uint8_t> bytes;
//
//                     REFLECT(token, logprob, bytes);
//                 };
//
//                 std::string token;
//                 double logprob;
//                 std::vector<std::uint8_t> bytes;
//                 std::vector<Logprob> top_logprobs;
//
//                 REFLECT(token, logprob, bytes, top_logprobs);
//             };
//
//             std::vector<Content> content;
//
//             REFLECT(content);
//         };
//
//         int index;
//         std::optional<Message> delta;
//         std::optional<Message> message;
//         std::optional<std::string> finish_reason;
//         std::optional<Logprobs> logprobs;
//
//         REFLECT(index, delta, finish_reason, logprobs);
//     };
//
//     struct Usage {
//         int completion_tokens;
//         int prompt_tokens;
//         int total_tokens;
//
//         REFLECT(completion_tokens, prompt_tokens, total_tokens);
//     };
//
//     std::string id;
//     std::vector<Choice> choices;
//     std::int64_t created;
//     std::string model;
//     std::optional<Usage> usage;
//
//     REFLECT(id, choices, created, model, usage);
// };
//
// static std::string form_code_complete(std::string_view code) {
//     std::string res;
//     res.append("Edit the following code to make it complete. Output the completed code without any additional text. Do not explain.\n");
//     /* res.append("```"); */
//     /* res.append(language); */
//     /* res.append("\n"); */
//     res.append(code);
//     /* res.append("\n"); */
//     /* res.append("```"); */
//     return res;
// }
//
// inline HTTPConnectionPool pool;
//
// static Task<Expected<OwningStream>> evaluate(std::string prompt) {
//     std::string authorization;
//     if (auto p = std::getenv("OPENAI_API_KEY")) {
//         authorization.append("Bearer ");
//         authorization.append(p);
//     }
//     auto conn = co_await co_await pool.connect("https://api.openai.com");
//     HTTPRequest req = {
//         .method = "POST",
//         .uri = URI::parse("/v1/chat/completions"),
//         .headers = {
//             {"content-type", "application/json"},
//             {"authorization", String{authorization}},
//         },
//     };
//     auto compReq = ChatCompletionRequest{
//         .messages = {
//             {.role = "user", .content = prompt},
//         },
//         .model = "gpt-3.5-turbo",
//         .max_tokens = 64,
//         .stream = true,
//     };
//     auto [res, body] = co_await co_await conn->request_streamed(req, json_encode(compReq));
//     auto [r, w] = pipe_stream();
//     co_spawn(pipe_bind(std::move(w), [conn = std::move(conn), body = std::move(body)] (OwningStream &w) mutable -> Task<Expected<>> {
//         while (auto tmp = co_await body.getline('\n')) {
//             std::string_view line = *tmp;
//             if (line.starts_with("data: "sv)) {
//                 line.remove_prefix(6);
//                 if (line == "[DONE]"sv) {
//                     break;
//                 }
//                 auto compRes = co_await json_decode<ChatCompletionResult>(line);
//                 auto delta = compRes.choices.at(0).delta.value().content.value_or("");
//                 co_await co_await w.putchunk(delta);
//             }
//         }
//         co_await co_await body.dropall();
//         co_return {};
//     }));
//     co_return std::move(r);
// }
//
// static void code_complete(std::string code, std::function<void(std::string)> callback) {
//     IOContext().join(co_bind([code = std::move(code), callback = std::move(callback)] () -> Task<Expected<>> {
//         auto prompt = form_code_complete(code);
//         auto stream = co_await co_await evaluate(std::move(prompt));
//         while (auto chunk = co_await stream.getchunk()) {
//             if (!chunk->empty()) {
//                 callback(std::string{std::move(*chunk)});
//             }
//         }
//         co_return {};
//     })).value();
// }
//
// int main() {
//     auto code = "#include <studio>\nint mian() {\nhow to print(hello world) here? help!\n}"s;
//     code_complete(code, [] (std::string chunk) {
//         std::cerr << chunk;
//     });
//     return 0;
// }
