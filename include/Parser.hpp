#ifndef PROFILE_INCLUDE_PARSER_HPP
#define PROFILE_INCLUDE_PARSER_HPP

#include <array>
#include <filesystem>
#include <vector>

namespace parser_info {

enum class CommentFormat : std::uint8_t {
  DoubleSlash,
  PoundSign,
  None,
};

class Parser {
 public:
  Parser(const std::vector<std::filesystem::path> files);
  void ListFiles() const;
  void ParseFiles() const;

 private:
  const bool IsValidFile(const std::filesystem::path& file,
                         CommentFormat& comment_format) const;
  const std::vector<std::filesystem::path> files_;
  static constexpr std::array<const char*, 8> double_slash_extensions_{
      ".c", ".cpp", ".h", ".hpp", ".js", ".rs", ".ts", ".zig"};
  static constexpr std::array<const char*, 1> pound_sign_extensions_{".py"};
};
}  // namespace parser_info
#endif  // PROFILE_INCLUDE_PARSER_HPP