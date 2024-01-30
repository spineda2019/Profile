#ifndef PROFILE_INCLUDE_PARSER_HPP
#define PROFILE_INCLUDE_PARSER_HPP

#include <array>
#include <filesystem>
#include <fstream>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace parser_info {

class UnexpectedFileTypeException : std::exception {
 public:
  UnexpectedFileTypeException(std::filesystem::path bad_file);
  const char* what() const noexcept;

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
  [[nodiscard]] int DocumentFiles(const std::filesystem::path& root_folder) const;

 private:
  const bool IsValidFile(const std::filesystem::path& file,
                         CommentFormat& comment_format) const;
  void RecursivelyParseFiles(const std::filesystem::path& current_file);

  void RecursivelyDocumentFiles(const std::filesystem::path& current_file, std::ofstream& output_markdown) const;

  std::size_t todo_count_;
  std::size_t fixme_count_;
  std::size_t file_count_;

  std::mutex print_lock_;

  static constexpr std::uint8_t FATAL_UNKNOWN_ERROR = 4;
  static constexpr std::uint8_t SUCCESS = 0;

  static constexpr std::array<const char*, 8> double_slash_extensions_{
      ".c", ".cpp", ".h", ".hpp", ".js", ".rs", ".ts", ".zig"};
  static constexpr std::array<const char*, 1> pound_sign_extensions_{".py"};
};

}  // namespace parser_info
#endif  // PROFILE_INCLUDE_PARSER_HPP