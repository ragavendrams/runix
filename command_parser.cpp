#include "command_parser.h"
#include <sstream>
#include <string>
#include <vector>

std::vector<const char *> CommandParser::parse(const std::string &command) {
  str_vec.clear();
  std::vector<const char *> result;

  std::string s;
  std::stringstream ss(command);

  while (std::getline(ss, s, ' ')) {
    str_vec.push_back(s);
  }

  for (const auto &str : str_vec) {
    result.push_back(str.c_str());
  }
  result.push_back(nullptr);

  return result;
}
