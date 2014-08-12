#pragma once

#include "enum_order.h"

#include <iosfwd>
#include <vector>
#include <utility>

/// Simple loader of the status protocol in the format, given in the exercise.
class protocol_loader
{
public:
  /// Read any kind of input stream of the protocol format, given in the exercise into a result
  /// vector. The format of a single protocol entry is (<time unit> <working order>).
  /// \return Vector of protocol entries.
  std::vector< std::pair<float,order> > read(std::istream& _istream);
};
