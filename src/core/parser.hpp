#ifndef PARSER_HPP
#define PARSER_HPP

#include <algorithm>
#include <string>

#include <functional>

#include "common.hpp"
#include "paramset.hpp"

/// Lambda expression that transform a c-style string to a lowercase c++-stype string version.
static auto str_to_lower = [](const char* c_str) -> std::string {
  std::string str{ c_str };
  std::transform(str.begin(), str.end(), str.begin(), ::tolower);
  return str;
};

// === Support functions

/// Map that binds a convertion function to an attribute name.
using ConverterFunction
  = std::function<bool(const std::string&, const std::string&, gc::ParamSet*)>;

// === parsing functions.
void parse_scene_file(const char*);
bool is_valid_tag(std::string_view tag_name);
bool is_valid_attribute(std::string_view tag_name, std::string_view attribute_name);
void parse_attribute(const std::string& attr_name,
                     const std::string& attr_content,
                     gc::ParamSet* ps);

#endif  // PARSER_HPP
