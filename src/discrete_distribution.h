#pragma once

#include <Eigen/Dense>

#include <cstdint>
#include <iosfwd>

/// A discrete probability distribution where the only observations are discrete observations.
/// Useful with Hidden Markov Models, where observations are non-negative integers representing
/// specific emissions.
/// This distribution only supports one-dimensional observations (observation_vector.cols() == 1).
/// Additional dimensions will simply be ignored.
/// \note While a discrete distribution only has positive integers (uint64_t) as observations they
///   can be converted to doubles, which is in internal use. The distribution will convert doubles
///   back into uint64_t before any kind of comparison.
class discrete_distribution
{
public:
  /// Default constructor, which creates a distribution that has no observations.
  discrete_distribution();

  /// Define the discrete distribution as having _observation_count possible observations. The
  /// probability in each state will default to ( 1 / _observation_count ).
  /// \param _observation_count number of possible observations this distribution can have.
  discrete_distribution(const std::uint64_t _observation_count);

  /// Define the discrete distribution as having the given probabilities for each observation.
  /// \param _probabilities probability of each possible observation.
  discrete_distribution(const Eigen::VectorXd& _probabilities);

  /// Get the dimensionality of the probability distribution. As the probability distribution is a
  /// simple vector it will always be 1.
  /// \return dimensionality of the probability distribution.
  std::uint64_t dimensionality() const { return 1; }

  /// Return the probability of the given observation. It should not be greater than the number of
  /// possible observations.
  /// \param _observation observation to return the probability of.
  /// \return probability of the given observation.
  double probability(const Eigen::VectorXd& _observation) const;

  /// Return a single, randomly generated observation as a single element of a one-dimensional
  /// vector, according to the probability distribution defined by this object.
  /// \return a random observation.
  Eigen::VectorXd random() const;

  /// Estimate the probability distribution directly from the given observations. It should not be
  /// greater than the number of possible observations.
  /// \param _observations list of observations.
  void estimate(const Eigen::MatrixXd& _observations);

  /// Estimate the probability distribution directly from the given observations. It should not be
  /// greater than the number of possible observations. Whether each observation is actually from
  /// this distribution is taken into account.
  /// \param _observations list of observations.
  /// \param _observations_probability list of probabilities that each observation is actually from
  ///   this distribution.
  void estimate(
    const Eigen::MatrixXd& _observations,
    const Eigen::VectorXd& _observations_probability
  );

  /// \return the vector of probabilities.
  const Eigen::VectorXd& probabilities() const { return m_probabilities; }

  /// Modify the vector of probabilities.
  /// \return reference to the vector of probabilities.
  Eigen::VectorXd& probabilities() { return m_probabilities; }

  /// Overload of the output operator for easy printing.
  /// \return a ostream representation of this distribution.
  friend std::ostream& operator<<(
    std::ostream& _ostream,
    discrete_distribution& _discrete_distribution
  );

private:
  /// Vector of probabilities in this distribution.
  Eigen::VectorXd m_probabilities;

  /// Normalize the vector of probabilities of this distribution.
  void normalize();

  /// Ensure that the observation is within the bounds of this distribution.
  /// \param _observation value of the observation.
  /// \param _index (optional) index within the list of observations.
  void check_observation(
    const std::uint64_t _observation,
    const std::uint64_t _index = 0
  ) const;
};
