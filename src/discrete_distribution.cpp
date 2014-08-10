#include "discrete_distribution.h"

#include <iostream>

DiscreteDistribution::DiscreteDistribution()
{ /* empty */ }

DiscreteDistribution::DiscreteDistribution(const std::uint64_t _observation_count)
{
  this->probabilities = Eigen::VectorXd::Ones(_observation_count) / static_cast<double>(_observation_count);
}

DiscreteDistribution::DiscreteDistribution(const Eigen::VectorXd& _probabilities)
{
  // make sure the probability distribution is normalized
  double sum = _probabilities.sum();
  if (sum > 0)
  {
    this->probabilities = _probabilities / sum;
  }
  else
  {
    // force normalization
    this->probabilities.resize(static_cast<std::uint64_t>(_probabilities.size()));
    this->probabilities.fill(1 / static_cast<double>(_probabilities.size()));
  }
}

double DiscreteDistribution::Probability(const Eigen::VectorXd& _observation) const
{
  const std::uint64_t obs = static_cast<std::uint64_t>(_observation[0]);

  check_observation(obs);

  return probabilities(obs);
}

Eigen::VectorXd DiscreteDistribution::Random() const
{
  // Generate a random number.
  //double random_observation = math::Random();
  double random_observation = 0.5;
  Eigen::VectorXd result(1);

  double sum_probability = 0;
  std::uint64_t max = this->probabilities.size();
  for (std::uint64_t observation = 0; observation < max; observation++)
  {
    if ((sum_probability += this->probabilities[observation]) >= random_observation)
    {
      result[0] = static_cast<double>(observation);
      return result;
    }
  }

  // this should never happen
  result[0] = static_cast<double>(max) - 1;
  return result;
}

void DiscreteDistribution::Estimate(const Eigen::MatrixXd& _observations)
{
  // clear probabilities
  this->probabilities.setZero();

  std::uint64_t max = _observations.cols();
  for (std::uint64_t i = 0; i < max; i++)
  {
    const std::uint64_t obs = static_cast<std::uint64_t>(_observations(0, i));

    check_observation(obs,i);

    // add the probability of each observation
    this->probabilities(obs)++;
  }

  normalize();
}

void DiscreteDistribution::Estimate(const Eigen::MatrixXd& _observations,
                                    const Eigen::VectorXd& _observations_probability)
{
  // clear probabilities
  this->probabilities.setZero();

  std::uint64_t max = _observations.cols();
  for (std::uint64_t i = 0; i < max; i++)
  {
    const std::uint64_t obs = static_cast<std::uint64_t>(_observations(0, i));

    check_observation(obs,i);

    // add the probability of each observation
    probabilities(obs) += _observations_probability[i];
  }

  normalize();
}

std::string DiscreteDistribution::ToString() const
{
  std::ostringstream convert;
  convert << "DiscreteDistribution [" << this << "]" << std::endl;
  convert << "Probabilities:" << std::endl;
  convert << probabilities;
  return convert.str();
}

// -------------------------------------------------------------------------------------------------
// private
// -------------------------------------------------------------------------------------------------

void DiscreteDistribution::normalize()
{
  // make sure the probability distribution is normalized
  double sum = this->probabilities.sum();
  if (sum > 0)
  {
    this->probabilities /= sum;
  }
  else
  {
    // force normalization
    this->probabilities.fill(1 / static_cast<double>(this->probabilities.size()));
  }
}

void DiscreteDistribution::check_observation(const std::uint64_t _observation, const std::uint64_t _index) const
{
  if (_observation >= static_cast<double>(this->probabilities.size()))
  {
    std::cout
      << "Observation " 
      << _index
      << " ("
      << _observation
      << ") must be in [0,"
      << this->probabilities.size()
      << "] for this distribution."
      << std::endl;
  }
}
