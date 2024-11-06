// benchmarks/url_benchmark.cpp
#include <benchmark/benchmark.h>
#include <seedlib/url.hpp>
#include <random>
#include <vector>

using namespace seedlib;

// Benchmark URL parsing
static void BM_URLParse(benchmark::State& state) {
    const std::string url = "https://example.com:8080/path?query=value#fragment";

    for (auto _ : state) {
        auto result = URL::parse(url);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_URLParse);

// Benchmark URL validation
static void BM_URLValidate(benchmark::State& state) {
    const std::string url = "https://example.com:8080/path?query=value#fragment";

    for (auto _ : state) {
        auto result = URL::validate(url);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_URLValidate);

// Benchmark URL toString
static void BM_URLToString(benchmark::State& state) {
    auto url = URL::parse("https://example.com:8080/path?query=value#fragment").value();

    for (auto _ : state) {
        std::string result = url.to_string();
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_URLToString);

// Benchmark with different URL lengths
static void BM_URLParse_Length(benchmark::State& state) {
    const int length = state.range(0);
    std::string url = "https://example.com/";
    url.append(length, 'a');

    for (auto _ : state) {
        auto result = URL::parse(url);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_URLParse_Length)->Range(8, 8<<10);

// Generate random URLs for throughput testing
static std::vector<std::string> generate_random_urls(size_t count) {
    std::vector<std::string> urls;
    urls.reserve(count);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> port_dist(1, 65535);

    const char* schemes[] = {"http", "https", "ws", "wss", "ftp"};
    const char* hosts[] = {"example.com", "localhost", "test.org", "demo.net"};
    const char* paths[] = {"/", "/api", "/v1/users", "/path/to/resource"};

    for (size_t i = 0; i < count; ++i) {
        std::stringstream ss;
        ss << schemes[i % 5] << "://"
           << hosts[i % 4] << ":"
           << port_dist(gen)
           << paths[i % 4];
        urls.push_back(ss.str());
    }

    return urls;
}

// Benchmark parsing throughput
static void BM_URLThroughput(benchmark::State& state) {
    const size_t url_count = 1000;
    auto urls = generate_random_urls(url_count);
    size_t index = 0;

    for (auto _ : state) {
        auto result = URL::parse(urls[index % url_count]);
        benchmark::DoNotOptimize(result);
        index++;
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_URLThroughput);

BENCHMARK_MAIN();
