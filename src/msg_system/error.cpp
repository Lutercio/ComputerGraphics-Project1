#include <cstdlib>  // std::exit
#include <iomanip>
#include <iostream>
#include <sstream>
#include "error.hpp"
#include "tcolor.hpp"
#include <string_view>

inline std::ostream& operator<<(std::ostream& os, const SourceContext& sc) {
  os << sc.file << ":" << sc.line;
  return os;
}
static constexpr short line_length{ 80 };

/// Prints out the warning, but the program keeps running.
void Warning(std::string_view msg, const SourceContext& sc) {
  std::ostringstream oss;
  oss << std::setw(line_length) << std::setfill('=') << " " << '\n'
      << "[WARNING]: " << msg << '\n'
      << "Reported at: <" << sc << "> \n"
      << std::setw(line_length) << std::setfill('=') << " " << '\n';

  std::cerr << TColor::colorize(oss.str(), TColor::YELLOW, TColor::BOLD);
}

/// Prints out the error message and exits the program.
void Error(std::string_view msg, const SourceContext& sc) {
  std::ostringstream oss;
  oss << std::setw(line_length) << std::setfill('=') << " " << '\n'
      << "[ERROR]: " << msg << '\n'
      << "Reported at: < " << sc << " > " << '\n'
      << "exiting...\n"
      << std::setw(line_length) << std::setfill('=') << " " << '\n';

  std::cerr << TColor::colorize(oss.str(), TColor::RED, TColor::BOLD);
  std::exit(EXIT_FAILURE);
}

void Message(std::string_view str) { std::cout << str << '\n'; }
