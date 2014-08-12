#include "protocol_loader.h"

#include <string>
#include <sstream>

std::vector< std::pair<float,order> > protocol_loader::read(std::istream& _istream)
{
  std::vector< std::pair<float,order> > protocol;

  if (!_istream) return protocol;

  float a;
  order b;

  for (std::string line; std::getline(_istream, line); )
  {
    std::istringstream iss(line);
    if (!(iss >> a >> b)) { break; } // error
    protocol.push_back(std::make_pair(a,b));
  }

  return protocol;
}
