/*
Copyright (c) 2024 Sebastian Pineda

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef INCLUDE_PARSER_HPP_
#define INCLUDE_PARSER_HPP_

#include <array>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <string>

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
  [[nodiscard]] int DocumentFiles(const std::filesystem::path& root_folder);

 private:
  const bool IsValidFile(const std::filesystem::path& file,
                         CommentFormat& comment_format) const;
  void RecursivelyParseFiles(const std::filesystem::path& current_file);

  void RecursivelyDocumentFiles(const std::filesystem::path& current_file,
                                std::ofstream& output_markdown);

  static std::size_t FindCommentPosition(
      const CommentFormat& comment_format, const std::string& line,
      const std::filesystem::path& current_file);

  static bool AreWeLookingForDocumentation(
      const std::string& line, const std::filesystem::path& current_file);

  std::size_t todo_count_;
  std::size_t fixme_count_;
  std::size_t file_count_;

  std::mutex print_lock_;
  std::mutex markdown_lock_;

  static constexpr std::uint8_t FATAL_UNKNOWN_ERROR = 2;
  static constexpr std::uint8_t FATAL_UNEXPECTED_FILETYPE_ERROR = 1;
  static constexpr std::uint8_t SUCCESS = 0;

  static constexpr std::array<const char*, 9> double_slash_extensions_{
      ".c", ".cpp", ".h", ".hpp", ".js", ".rs", ".ts", ".zig", ".cs"};
  static constexpr std::array<const char*, 1> pound_sign_extensions_{".py"};
};

}  // namespace parser_info
#endif  // INCLUDE_PARSER_HPP_
