#ifndef PROFILE_INCLUDE_PARSER_HPP
#define PROFILE_INCLUDE_PARSER_HPP

#include <array>
#include <filesystem>
#include <vector>

namespace parser_info {

enum CommentFormat {
  DoubleSlash,
  PoundSign,
};

class Parser {
 public:
  Parser(const std::vector<std::filesystem::path> files);
  void ListFiles() const;
  void ParseFiles() const;

 private:
  const bool IsValidFile(const std::filesystem::path& file) const;
  const std::vector<std::filesystem::path> files_;
  static constexpr std::array<const char*, 8> double_slash_extensions_{
      ".c", ".cpp", ".h", ".hpp", ".js", ".rs", ".ts", ".zig"};
  static constexpr std::array<const char*, 1> pound_sign_extensions_{".py"};
};
}  // namespace parser_info
#endif  // PROFILE_INCLUDE_PARSER_HPP