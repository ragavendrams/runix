#include <sstream>
#include <string>
#include <vector>

class CommandParser {
private:
  std::vector<std::string> str_vec;

public:
  // Default ctor, dtor
  std::vector<const char *> parse(const std::string &command);
};