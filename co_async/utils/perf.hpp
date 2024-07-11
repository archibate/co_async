#pragma once
#if CO_ASYNC_PERF
# include <co_async/std.hpp>

namespace co_async {
struct Perf {
private:
    char const *file;
    std::uint_least32_t line;
    std::chrono::steady_clock::time_point t0;

    struct PerfTableEntry {
        std::uint64_t duration;
        char const *file;
        int line;
    };

    struct PerfThreadLocal {
        std::deque<PerfTableEntry> table;
        PerfThreadLocal() = default;
        PerfThreadLocal(PerfThreadLocal &&) = delete;

        ~PerfThreadLocal() {
            gather(*this);
        }
    };

    struct PerfGather {
        /* PerfGather() { */
        /*     signal( */
        /*         SIGINT, +[](int signo) { std::exit(130); }); */
        /* } */

        PerfGather &operator=(PerfGather &&) = delete;

        void dump() const {
            if (table.empty()) {
                return;
            }

            struct PairLess {
                bool
                operator()(std::pair<std::string_view, int> const &a,
                           std::pair<std::string_view, int> const &b) const {
                    return std::tie(a.first, a.second) <
                           std::tie(b.first, b.second);
                }
            };

            struct Entry {
                std::uint64_t min = std::numeric_limits<std::uint64_t>::max();
                std::uint64_t sum = 0;
                std::uint64_t max = 0;
                std::uint64_t nr = 0;

                Entry &operator+=(std::uint64_t d) {
                    min = std::min(min, d);
                    sum += d;
                    max = std::max(max, d);
                    ++nr;
                    return *this;
                }
            };

            std::map<std::pair<std::string_view, int>, Entry, PairLess> m;
            for (auto const &e: table) {
                m[{e.file, e.line}] += e.duration;
            }
            auto t = [](std::uint64_t d) -> std::string {
                if (d < 10000) {
                    return std::format("{}ns", d);
                } else if (d < 10'000'000) {
                    return std::format("{}us", d / 1000);
                } else if (d < 10'000'000'000) {
                    return std::format("{}ms", d / 1'000'000);
                } else if (d < 10'000'000'000'000) {
                    return std::format("{}s", d / 1'000'000'000);
                } else {
                    return std::format("{}h", d / 3'600'000'000'000);
                }
            };
            auto p = [](std::string_view s) -> std::string {
                auto p = s.rfind('/');
                if (p == std::string_view::npos) {
                    return std::string(s);
                } else {
                    return std::string(s.substr(p + 1));
                }
            };
            std::vector<std::pair<std::pair<std::string_view, int>, Entry>>
                sorted(m.begin(), m.end());
            std::sort(sorted.begin(), sorted.end(),
                      [](auto const &lhs, auto const &rhs) {
                          return lhs.second.sum > rhs.second.sum;
                      });
            std::size_t w = 0, nw = 1;
            for (auto const &[loc, e]: sorted) {
                w = std::max(w, p(loc.first).size());
                nw = std::max(nw, std::to_string(e.nr).size());
            }
            std::string o;
            auto oit = std::back_inserter(o);
            std::format_to(oit, "{:>{}}:{:<4} {:^6} {:^6} {:^6} {:^6} {:^{}}\n",
                           "file", w, "line", "min", "avg", "max", "sum", "nr",
                           nw + 1);
            for (auto const &[loc, e]: sorted) {
                std::format_to(oit,
                               "{:>{}}:{:<4} {:>6} {:>6} {:>6} {:>6} {:>{}}x\n",
                               p(loc.first), w, loc.second, t(e.min),
                               t(e.sum / e.nr), t(e.max), t(e.sum), e.nr, nw);
            }
            fprintf(stderr, "%s", o.c_str());
        }

        ~PerfGather() {
            for (auto *thread: threads) {
                gather(*thread);
            }
            dump();
        }

        std::deque<PerfTableEntry> table;
        std::set<PerfThreadLocal *> threads;
        std::mutex lock;
    };

    static inline PerfGather gathered;
    static inline thread_local PerfThreadLocal perthread;

    static void gather(PerfThreadLocal &perthread) {
        std::lock_guard guard(gathered.lock);
        gathered.table.insert(gathered.table.end(), perthread.table.begin(),
                              perthread.table.end());
        gathered.threads.erase(&perthread);
    }

public:
    Perf(std::source_location loc = std::source_location::current())
        : file(loc.file_name()),
          line(loc.line()),
          t0(std::chrono::steady_clock::now()) {}

    Perf(Perf &&) = delete;

    ~Perf() {
        auto t1 = std::chrono::steady_clock::now();
        auto duration = (t1 - t0).count();
        perthread.table.emplace_back(duration, file, line);
    }
};
} // namespace co_async
#else
namespace co_async {
struct Perf {};
} // namespace co_async
#endif
