#pragma once

#include <cstdint>
#include <cstdlib>
#include <random>
#include <cmath>

namespace math {

extern std::random_device random_device;
extern std::mt19937_64 engine;
extern std::uniform_real_distribution<> uniform_real_distribution;
extern std::normal_distribution<> normal_distribution;

/// Define a seed for general pseudo random number generation.
inline void seed(const std::uint64_t _seed)
{
  std::uint32_t seed = static_cast<std::uint32_t>(_seed);
  engine.seed(seed);
  std::srand(seed);
}

/// Generate a uniform random number between zero and one.
inline double random()
{
  return uniform_real_distribution(engine);
}

/// Generate a uniform random number within the specified range.
/// \param min minimum of the value range.
/// \param max maximum of the value range.
inline double random(const double min, const double max)
{
  return min + (max - min) * random();
}

/// Generate a normally distributed random number with (mean = 0) and (variance = 1).
inline double random_normal()
{
  return normal_distribution(engine);
}

/// Generate a normally distributed random number with the specified mean and variance.
/// \param mean mean of the distribution.
/// \param variance variance of the distribution.
inline double random_normal(const double mean, const double variance)
{
  return variance * random_normal() + mean;
}

/// Generate a uniform random integer with the specified maximal value.
/// \param max maximum of the integer value.
inline int random_int(const int max)
{
  return static_cast<int>(std::floor(max * random()));
}

/// Generate a uniform random integer within the specified range.
/// \param min minimum of the integer value range.
/// \param max maximum of the integer value range.
inline int random_int(const int min, const int max)
{
  return min + static_cast<int>(std::floor((max - min) * random()));
}

}; // namespace math
