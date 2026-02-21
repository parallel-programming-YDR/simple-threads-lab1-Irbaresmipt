#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <ostream>
#include <random>
#include <thread>
#include <vector>

using AreaT   = double;
using Integer = std::int64_t;
std::atomic_int64_t num_hits{};

void CircleArea(Integer num_tries, AreaT radius, Integer seed)
{
  std::mt19937_64                        gen(seed);
  std::uniform_real_distribution<double> coord(-radius, radius);

  Integer points_inside{};
  AreaT   x{}, y{};
  AreaT   radius_sq = radius * radius;

  for (Integer i{}; i < num_tries; ++i)
  {
    x = coord(gen);
    y = coord(gen);

    if (x * x + y * y <= radius_sq)
    {
      ++points_inside;
    }
  }

  num_hits.fetch_add(points_inside, std::memory_order_relaxed);
}

int main(int argc, char *argv[])
{
  if (argc < 2 || argc > 3)
  {
    std::cerr << "Error: Usage: " << argv[0] << " tries [seed]\n";
    return EXIT_FAILURE;
  }

  Integer num_tries{}, seed{};

  num_tries = std::stoll(argv[1]);

  if (num_tries <= 0)
  {
    std::cerr << "Error: Tries num should be positive\n";
    return EXIT_FAILURE;
  }

  if (argc == 3)
  {
    seed = std::stoll(argv[2]);
  }

  if (seed < 0)
  {
    std::cerr << "Error: Seed should be non-negative\n";
    return EXIT_FAILURE;
  }

  AreaT   radius{};
  Integer num_threads{};

  while (std::cin >> radius >> num_threads)
  {

    if (radius <= 0 || num_threads <= 0)
    {
      std::cerr << "Error: Radius and number of threads should be positive\n";
      return EXIT_FAILURE;
    }
    auto start = std::chrono::high_resolution_clock::now();

    if (num_threads > std::thread::hardware_concurrency())
    {
      std::cout << "Warning! Exedeed hardware concurrency! New threads do not make efficiency increase!\n";
    }

    std::vector<std::thread> threads;
    threads.reserve(num_threads);

    Integer base_tries = num_tries / num_threads;
    Integer remainder  = num_tries % num_threads;

    for (std::int64_t i{}; i < num_threads; ++i)
    {

      threads.emplace_back(CircleArea, base_tries + (i < remainder ? 1 : 0), radius, seed + i);
    }

    // Waiting for work finished
    for (auto &t : threads)
    {
      t.join();
    }

    auto end = std::chrono::high_resolution_clock::now();

    std::cout << std::fixed << std::setprecision(3) << std::chrono::duration<double, std::milli>(end - start).count()
              << " "
              << 4 * radius * radius * (num_hits.load(std::memory_order_relaxed) / static_cast<long double>(num_tries))
              << std::endl;

    num_hits.store(0, std::memory_order_relaxed); // clear
  }

  return EXIT_SUCCESS;
}
