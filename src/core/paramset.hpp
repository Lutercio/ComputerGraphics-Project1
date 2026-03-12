/*!
 * This class implements an heterogeneous dictionary.
 * @author Selan
 * @data 2025
 */

#ifndef PARAMSET_HPP
#define PARAMSET_HPP

#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <typeinfo>
#include "common.hpp"

namespace gc {
/// This is the **base** class that represents a generic value.
/*!
 * This is a pure virtual basic type.
 * The data part of the map inside ParamSet stores a shared pointer to this base class.
 */
class GenericType {
public:
  GenericType() = default;
  virtual ~GenericType() = default;
};

/// This is a **derived** class that actually stores a template value.
/*!
 * This class is derived from `GenericType`.
 * It works as a wrap around a template value.
 * We rely on polymorphism to cast a `GenericType` pointer into
 * a `ValueType<T>` object that holds the `T` value the client wants.
 * This way, we may have an heterogeneous map.
 */
template <typename T>
class ValueType : public GenericType {
public:
  T m_value;  // The stored data.
  ValueType(const T& value = T{}) : m_value{ value } {}
  ~ValueType() override = default;
};

/*!
 * This class implements an heterogeneous dictionary of key-value items.
 * The `key` in the ParamSet class is always a string.
 * The `value` may be of different data types.
 *
 * Because we need to store values of different types in this dictionary,
 * we associate the key (string) to a **pointer** to a generic type (polymorphism).
 * We move from this generic reference back to a specific type only when we
 * try to retrieve a value from the dictionary via method `retrieve()`.
 */
class ParamSet {
  using Dictionary = std::map<std::string, std::shared_ptr<GenericType>>;
  Dictionary m_map;  //!< This is where the key-value items are stored.

public:
  //= Regular methods.
  ParamSet() = default;
  ~ParamSet() = default;

  /*!
   * Tries to retrieve the value associated with `key`.
   * If there is no such key/value pair in the dictionary, the method returns
   * the default value `default_value` passed via arguments.
   *
   * @param key The key stored in the ParamSet.
   * @param default_value The default value returned, in case key is not in the ParamSet.
   * @return The value associated with `key`, if such key is stored in the `ps`, or
   * the given default value otherwise.
   */
  template <typename T>
  T retrieve(const std::string& key, const T& default_value = T{}) const {
    // Try to retrieve key/data item from the map.
    auto result = m_map.find(key);
    if (result == m_map.end()) {  // Not found!
      return default_value;
    }
    // Get key-value pair from iterator.
    const auto& [the_key, sptr] = *result;
    // Convert generic pointer to the base value of type T.
    try {
      auto dummy = dynamic_cast<ValueType<T>&>(*sptr);
    } catch (const std::bad_cast& e) {
      std::cout << "Received a bad_cast exception while trying to convert " << std::quoted(the_key)
                << "\n";
    }
    auto data = dynamic_cast<ValueType<T>&>(*sptr);
    return data.m_value;  // Finally, return the stored value.
  }

  /*!
   * Assigns a pair key-value to the ParamSet.
   * If the key already exists, the stored value is overwritten by the new data value provided.
   * @param key The key associated with the `value`.
   * @param value The value the client wants to store.
   */
  template <typename T>
  void assign(const std::string& key, const T& value) {
    m_map[key] = std::make_shared<ValueType<T>>(value);
  }

  /*!
   * Checks if a given key is present in the ParamSet bundle.
   * @param key The key we want to investigate.
   * @return true if the key is present; false otherwise.
   */
  template <typename T>
  [[nodiscard]] bool contains(const std::string& key) const {
    return m_map.count(key) == 1;
  }
};

}  // namespace gc

#endif
