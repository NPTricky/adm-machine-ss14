#include "random.h"

namespace math {

std::random_device random_device;
std::mt19937_64 engine(random_device());
std::uniform_real_distribution<> uniform_real_distribution(0,1);
std::normal_distribution<> normal_distribution(0,1);

}; // namespace math