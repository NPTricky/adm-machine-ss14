#pragma once

#include "create_map.h"

#include <cstdint>
#include <map>
#include <string>
#include <iosfwd>

enum class order : std::uint8_t { 
  D,
  OK,
};

static std::map<std::string, order> map_string_to_order = create_map<std::string, order>("D",order::D)("OK",order::OK);
static std::map<order, std::string> map_order_to_string = create_map<order, std::string>(order::D,"D")(order::OK,"OK");

inline order string_to_order(const std::string _order)
{
  return map_string_to_order[_order];
}

inline std::string order_to_string(const order _order)
{
  return map_order_to_string[_order];
}

inline std::ostream& operator<<(std::ostream& _ostream, const order& _order)
{
  _ostream << order_to_string(_order);
  return _ostream;
}

inline std::istream& operator>>(std::istream& _istream, order& _order) {
  std::string word;
  _istream >> word;
  _order = string_to_order(word);        
  return _istream;      
}
