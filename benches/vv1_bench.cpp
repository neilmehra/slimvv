#include "../include/vv1.hpp"
#include <benchmark/benchmark.h>
#include <cstdlib>
#include <limits>
#include <random>
#include <vector>

namespace bm = benchmark;

constexpr std::size_t num_iter = 5000;

struct BigType {
  char c[5000];
};

void bench_pushback(bm::State& state) {
  std::mt19937 gen{std::random_device{}()};
  std::uniform_int_distribution<> num_dist(0, std::numeric_limits<int>::max());
  int arg0 = num_dist(gen);
  long long arg1 = num_dist(gen);
  BigType arg2{};

  std::vector<int> types(num_iter);
  std::uniform_int_distribution<> t_dist(0, 2);
  for (std::size_t i = 0; i < num_iter; i++) {
    types[i] = t_dist(gen);
  }

  v1::vector<int, long long, BigType> v;

  for (auto _ : state) {
    for (std::size_t i = 0; i < num_iter; i++) {
      if (types[i] == 0) {
        v.push_back(arg0);
      } else if (types[i] == 1) {
        v.push_back(arg1);
      } else {
        v.push_back(arg2);
      }
    }

    bm::DoNotOptimize(v);
  }
}

void bench_index(bm::State& state) {
  std::mt19937 gen{std::random_device{}()};
  std::uniform_int_distribution<> num_dist(0, std::numeric_limits<int>::max());
  int arg0 = num_dist(gen);
  long long arg1 = num_dist(gen);
  BigType arg2{};

  std::vector<int> types(num_iter);
  std::uniform_int_distribution<> t_dist(0, 2);
  for (std::size_t i = 0; i < num_iter; i++) {
    types[i] = t_dist(gen);
  }

  v1::vector<int, long long, BigType> v;
  for (std::size_t i = 0; i < num_iter; i++) {
    if (types[i] == 0) {
      v.push_back(arg0);
    } else if (types[i] == 1) {
      v.push_back(arg1);
    } else {
      v.push_back(arg2);
    }
  }

  for (auto _ : state) {
    for (std::size_t i = 0; i < num_iter; i++) {
      bm::DoNotOptimize(v[i]);
    }
  }
}

BENCHMARK(bench_pushback)->Unit(bm::kMillisecond);
BENCHMARK(bench_index)->Unit(bm::kMillisecond);
BENCHMARK_MAIN();
