#pragma once

#include <map>

/// Generic class for the chained creation of a std::map.
template <typename K, typename V>
class create_map
{
public:
  /// Constructor for a map with a single entry.
  /// \param _key
  /// \param _value
  create_map(const K& _key, const V& _value)
  {
    m_map[_key] = _value;
  }

  /// Operator() for chaining of key-value pairs.
  /// \param _key
  /// \param _value
  create_map<K, V>& operator()(const K& _key, const V& _value)
  {
    m_map[_key] = _value;
    return *this;
  }

  operator std::map<K, V>()
  {
    return m_map;
  }

private:
  std::map<K, V> m_map;

};
