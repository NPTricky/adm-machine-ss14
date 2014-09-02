#pragma once

#include "discrete_distribution.h"

#include <Eigen/Dense>

#include <complex>
#include <vector>
#include <cstdint>
#include <iostream>
#include <iomanip>

template<typename distribution = discrete_distribution>
class markov_chain
{
public:

  /// Constructor of a default, uniformly distributed markov chain.
  /// \param _state_count Total number of states.
  /// \param _emission_distribution A single emission probability distribution. It will be used
  ///   the same for every state.
  markov_chain(
    const std::uint64_t _state_count,
    const distribution& _emission_distribution
  );

  /// Constructor of a markov chain with the given initial state vector and its transition matrix
  /// \param _initial_state Vector of initial state probabilities.
  /// \param _transition_matrix Matrix of transition probabilities.
  /// \param _emission_distributions Vector of emission probability distributions. One for each
  ///   state.
  markov_chain(
    const Eigen::RowVectorXd& _initial_state,
    const Eigen::MatrixXd& _transition_matrix,
    const std::vector<distribution>& _emission_distributions
  );

  /// Conversion of the generator matrix of a continuous time markov chain (CTMC) with the given
  /// time step into a discrete transition matrix.
  /// \param _generator_matrix Matrix of expected values for a CTMC.
  /// \param _delta_t Size of a single, discrete time step.
  static Eigen::MatrixXd from_ctmc(const Eigen::MatrixXd& _generator_matrix, const double _delta_t);

  /// Estimate the state vector after the specified number of steps or if the rate of change
  /// threshold requirements are met. With default parameters the function tries to converge on a
  /// stationary solution.
  /// Uses the basic, iterative algorithm.
  /// \param _steps Number of steps to evaluate.
  /// \param _epsilon Threshold for the rate of change of the state vector between each step,
  ///   measured in euclidean distance. Set epsilon to a negative value to deactivate its break
  ///   condition. An epsilon of zero achieves the maximum available numerical precision obtainable
  ///   by the specific hard- & software.
  void estimate(
    const std::uint64_t _steps = std::numeric_limits<uint64_t>::max(),
    const double _epsilon = 0
  );

  /// Estimate the state vector after the specified number of steps or if the rate of change
  /// threshold requirements are met. With default parameters the function tries to converge on a
  /// stationary solution.
  /// Uses the power method.
  /// \param _steps Number of steps to evaluate.
  /// \param _epsilon Threshold for the rate of change of the state vector between each step,
  ///   measured in euclidean distance. Set epsilon to a negative value to deactivate its break
  ///   condition. An epsilon of zero achieves the maximum available numerical precision obtainable
  ///   by the specific hard- & software. Tests have proven numerically unstable at
  ///   epsilon < 1.0e-12.
  void estimate_power(
    const std::uint64_t _steps = std::numeric_limits<uint64_t>::max(),
    const double _epsilon = 1.0e-8
  );

private:
  /// Initial state vector.
  Eigen::RowVectorXd m_initial_state;

  /// Transition probability matrix.
  Eigen::MatrixXd m_transition_matrix;

  /// Vector of emission probability distributions. One for each state.
  std::vector<distribution> m_emission_distributions;

};

// -------------------------------------------------------------------------------------------------
// implementation
// -------------------------------------------------------------------------------------------------

template<typename distribution/* = discrete_distribution */>
markov_chain<distribution>::markov_chain(
  const std::uint64_t _state_count,
  const distribution& _emission_distribution
) : m_initial_state(Eigen::RowVectorXd::Ones(_state_count) / static_cast<double>(_state_count))
  , m_transition_matrix(Eigen::MatrixXd::Ones(_state_count,_state_count) / static_cast<double>(_state_count))
  , m_emission_distributions(_state_count,_emission_distribution)
{
  /* empty */
}

template<typename distribution/* = discrete_distribution */>
markov_chain<distribution>::markov_chain(
  const Eigen::RowVectorXd& _initial_state,
  const Eigen::MatrixXd& _transition_matrix,
  const std::vector<distribution>& _emission_distributions
) : m_initial_state(_initial_state)
  , m_transition_matrix(_transition_matrix)
  , m_emission_distributions(_emission_distributions)
{
  /* empty */
}

template<typename distribution/* = discrete_distribution */>
Eigen::MatrixXd markov_chain<distribution>::from_ctmc(const Eigen::MatrixXd& _generator_matrix, const double _delta_t)
{
  Eigen::MatrixXd identity_matrix = Eigen::MatrixXd::Identity(_generator_matrix.rows(),_generator_matrix.cols());
  return identity_matrix + _delta_t * _generator_matrix;
}

template<typename distribution/* = discrete_distribution */>
void markov_chain<distribution>::estimate(const std::uint64_t _steps, const double _epsilon)
{
  auto current = m_initial_state; // row vector

  std::uint64_t i;
  for (i = 0; i < _steps; i++)
  {
    auto last = current;
    current = last * m_transition_matrix;
    auto difference = current - last;
    auto distance = difference.norm();

    if (distance <= _epsilon) { break; }
  }

  std::cout << std::setprecision(10) << "estimate " << current << " after " << i << " steps with a precision of " << _epsilon << "." << std::endl;
}

template<typename distribution/* = discrete_distribution */>
void markov_chain<distribution>::estimate_power(const std::uint64_t _steps, const double _epsilon)
{
  auto current = m_transition_matrix; // square matrix

  std::uint64_t i;
  for (i = 0; i < _steps; i++)
  {
    auto last = current;
    // another way to efficient, precise runtime solutions of pow(matrix,n) without the danger of
    // aliasing would be a constexpr or expression templates
    current = last * last;
    auto difference = current.row(0) - last.row(0);
    auto distance = difference.norm();

    if (distance <= _epsilon) { break; }
  }

  std::cout << std::setprecision(10) << "estimate " << current.row(0) << " after " << i << " steps with a precision of " << _epsilon << "." << std::endl;
}
