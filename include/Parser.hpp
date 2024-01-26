#ifndef PROFILE_INCLUDE_PARSER_HPP
#define PROFILE_INCLUDE_PARSER_HPP

#include <array>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <vector>
#include <thread>

namespace parser_info {

class UnexpectedFileTypeException : std::exception {
 public:
  UnexpectedFileTypeException(std::filesystem::path bad_file);
  const char* what() noexcept;

 private:
  std::filesystem::path bad_file_;
};

enum class CommentFormat : std::uint8_t {
  DoubleSlash,
  PoundSign,
  None,
};

class Parser {
 public:
  Parser();
  [[nodiscard]] int ParseFiles(const std::filesystem::path& current_file);

 private:
  const bool IsValidFile(const std::filesystem::path& file);
  void RecursivelyParseFiles(const std::filesystem::path& current_file);

  std::unique_ptr<std::vector<std::thread>> directory_threads_;

  std::fstream file_stream_;
  std::string line_;

  std::size_t line_count_;
  std::size_t todo_count_;
  std::size_t fixme_count_;
  std::size_t file_count_;

  std::size_t todo_position_;
  std::size_t fixme_position_;
  std::size_t comment_position_;

  CommentFormat comment_format_;

  static constexpr std::uint8_t FATAL_UNKNOWN_ERROR = 4;
  static constexpr std::uint8_t FATAL_UNEXPECTED_FILETYPE = 3;
  static constexpr std::uint8_t INVALID_FILE_FOUND = 1;
  static constexpr std::uint8_t EXISTING_SYMLINK_FOUND = 2;

  static constexpr std::array<const char*, 8> double_slash_extensions_{
      ".c", ".cpp", ".h", ".hpp", ".js", ".rs", ".ts", ".zig"};
  static constexpr std::array<const char*, 1> pound_sign_extensions_{".py"};
};

}  // namespace parser_info
#endif  // PROFILE_INCLUDE_PARSER_HPP