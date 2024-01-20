#ifndef PROFILE_INCLUDE_PARSER_HPP
#define PROFILE_INCLUDE_PARSER_HPP

#include <filesystem>
#include <vector>

class Parser {
 public:
  static Parser Create(const std::vector<std::filesystem::path> files);
  void ListFiles() const;

 private:
  Parser(const std::vector<std::filesystem::path> files);
  const std::vector<std::filesystem::path> files_;
};
#endif  // PROFILE_INCLUDE_PARSER_HPP