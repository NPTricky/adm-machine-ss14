#include "discrete_distribution.h"
#include "random.h"

#include <iostream>

// -------------------------------------------------------------------------------------------------
// public
// -------------------------------------------------------------------------------------------------

discrete_distribution::discrete_distribution()
{ /* empty */ }

discrete_distribution::discrete_distribution(const std::uint64_t _observation_count)
{
  m_probabilities = Eigen::VectorXd::Ones(_observation_count) / static_cast<double>(_observation_count);
}

discrete_distribution::discrete_distribution(const Eigen::VectorXd& _probabilities)
{
  // make sure the probability distribution is normalized
  double sum = _probabilities.sum();
  if (sum > 0)
  {
    m_probabilities = _probabilities / sum;
  }
  else
  {
    // force normalization
    m_probabilities.resize(static_cast<std::uint64_t>(_probabilities.size()));
    m_probabilities.fill(1 / static_cast<double>(_probabilities.size()));
  }
}

double discrete_distribution::probability(const Eigen::VectorXd& _observation) const
{
  const std::uint64_t obs = static_cast<std::uint64_t>(_observation[0]);

  check_observation(obs);

  return m_probabilities(obs);
}

Eigen::VectorXd discrete_distribution::random() const
{
  // generate a random, uniformly distributed number
  double random_observation = math::random();
  Eigen::VectorXd result(1);

  double sum_probability = 0;
  std::uint64_t max = m_probabilities.size();
  for (std::uint64_t observation = 0; observation < max; observation++)
  {
    if ((sum_probability += m_probabilities[observation]) >= random_observation)
    {
      result[0] = static_cast<double>(observation);
      return result;
    }
  }

  // this should never happen
  result[0] = static_cast<double>(max) - 1;
  return result;
}

void discrete_distribution::estimate(const Eigen::MatrixXd& _observations)
{
  // clear probabilities
  m_probabilities.setZero();

  std::uint64_t max = _observations.cols();
  for (std::uint64_t i = 0; i < max; i++)
  {
    const std::uint64_t obs = static_cast<std::uint64_t>(_observations(0, i));

    check_observation(obs,i);

    // add the probability of each observation
    m_probabilities(obs)++;
  }

  normalize();
}

void discrete_distribution::estimate(
  const Eigen::MatrixXd& _observations,
  const Eigen::VectorXd& _observations_probability)
{
  // clear probabilities
  m_probabilities.setZero();

  std::uint64_t max = _observations.cols();
  for (std::uint64_t i = 0; i < max; i++)
  {
    const std::uint64_t obs = static_cast<std::uint64_t>(_observations(0, i));

    check_observation(obs,i);

    // add the probability of each observation
    m_probabilities(obs) += _observations_probability[i];
  }

  normalize();
}

// -------------------------------------------------------------------------------------------------
// private
// -------------------------------------------------------------------------------------------------

void discrete_distribution::normalize()
{
  // make sure the probability distribution is normalized
  double sum = m_probabilities.sum();
  if (sum > 0)
  {
    m_probabilities /= sum;
  }
  else
  {
    // force normalization
    m_probabilities.fill(1 / static_cast<double>(m_probabilities.size()));
  }
}

void discrete_distribution::check_observation(
  const std::uint64_t _observation,
  const std::uint64_t _index) const
{
  if (_observation >= static_cast<double>(m_probabilities.size()))
  {
    std::cout
      << "Observation " 
      << _index
      << " ("
      << _observation
      << ") must be in [0,"
      << m_probabilities.size()
      << "] for this distribution."
      << std::endl;
  }
}

// -------------------------------------------------------------------------------------------------
// friend
// -------------------------------------------------------------------------------------------------

std::ostream& operator<<(
  std::ostream& _ostream,
  discrete_distribution& _discrete_distribution)
{
  _ostream
    << "discrete_distribution ["
    << _discrete_distribution.m_probabilities
    << "]\n";
  return _ostream;
}