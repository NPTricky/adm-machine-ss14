#include "create_map.h"

#include <cstdint>
#include <map>
#include <string>
#include <iosfwd>

enum class order : std::uint8_t { 
  D,
  OK,
};

std::map<std::string, order> order_name = create_map<std::string, order>("D",order::D)("OK",order::OK);

const char* order_to_string(const order _order)
{
  switch (_order)
  {
  case order::D:
    return "D";
  case order::OK:
    return "OK";
  default:
    return "?";
  }
}

order string_to_order(const char* _order)
{
  return order_name[_order];
}

std::ostream& operator<<(std::ostream& _ostream, const order& _order)
{
  _ostream << order_to_string(_order);
  return _ostream;
}

std::istream& operator>>(std::istream& _istream, order& _order) {
  std::string word;
  _istream >> word;
  _order = string_to_order(word.c_str());        
  return _istream;      
}
