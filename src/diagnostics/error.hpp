#ifndef ERROR_H
# define ERROR_H

# include <string_view>

# define SC           SourceContext(__FILE__, __LINE__)
# define ERROR(msg)   Error(msg, SC)
# define WARNING(msg) Warning(msg, SC)
# define MESSAGE(msg) Message(msg)

// Holds context information for a warning or error while pre-processing scene file or executing the
// renderer. Use macro SC to create one
struct SourceContext {
  const char* file;
  int line;
  SourceContext(const char* file = "n/a", int line = 0) : file(file), line(line) {}
};

/// Prints out the error message and exits the program.
void Error(std::string_view, const SourceContext&);
/// Prints out the warning, but the program keeps running.
void Warning(std::string_view, const SourceContext&);
/// Prints out a simple message, program keeps running.
void Message(std::string_view);

#endif
//-------------------------------------------------------------------------------
