#include "enum_order.h"

#include <iosfwd>
#include <vector>
#include <string>

class loader
{
public:
  void read(std::istream& _istream);
  void write(std::ostream& _ostream);
};

void loader::read(std::istream& _istream)
{
  if (!_istream) return;

  float a;
  order b;
  std::vector< std::pair<float,order> > protocol;

  for (std::string line; std::getline(_istream, line); )
  {
    std::istringstream iss(line);
    if (!(iss >> a >> b)) { break; } // error
    protocol.push_back(std::make_pair(a,b));
  }
}
