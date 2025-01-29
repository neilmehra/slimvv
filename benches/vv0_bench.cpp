#include "../include/vv0.hpp"
#include <benchmark/benchmark.h>
#include <cstdlib>
#include <vector>

namespace bm = benchmark;

constexpr std::size_t num_iter = 5000;

struct BigType {
  char c[5000];
};

void bench_pushback(bm::State& state) {
  BigType arg{};
  v0::vector<int, long long, BigType> v;

  for (auto _ : state) {
    for (std::size_t i = 0; i < num_iter; i++) {
      v.push_back(arg);
    }

    bm::DoNotOptimize(v);
  }
}

void bench_index(bm::State& state) {
  BigType arg{};
  v0::vector<int, long long, BigType> v;
  for (std::size_t i = 0; i < num_iter; i++) {
    v.push_back(arg);
  }

  for (auto _ : state) {
    for (std::size_t i = 0; i < num_iter; i++) {
      bm::DoNotOptimize(std::get<BigType>(v[i]));
    }
  }
}

BENCHMARK(bench_pushback)->Unit(bm::kMillisecond);
BENCHMARK(bench_index)->Unit(bm::kMillisecond);
BENCHMARK_MAIN();
