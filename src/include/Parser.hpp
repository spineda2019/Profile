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
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace parser_info {

class UnexpectedFileTypeException : std::exception {
 public:
  explicit UnexpectedFileTypeException(std::filesystem::path bad_file);
  const char* what() const noexcept;

 private:
  std::filesystem::path bad_file_;
};

enum class CommentFormat : std::uint8_t {
  DoubleSlash,
  PoundSign,
};

class Parser {
 public:
  Parser();
  [[nodiscard]] int ParseFiles(
      const std::filesystem::path& current_file) noexcept;
  [[nodiscard]] int DocumentFiles(const std::filesystem::path& root_folder);

 private:
  const std::optional<CommentFormat> IsValidFile(
      const std::filesystem::path& file) const;
  void RecursivelyParseFiles(const std::filesystem::path& current_file);

  void RecursivelyDocumentFiles(const std::filesystem::path& current_file,
                                std::ofstream& output_markdown);

  static std::optional<std::size_t> FindCommentPosition(
      const std::optional<CommentFormat>& comment_format,
      const std::string_view line, const std::filesystem::path& current_file);

  static bool AreWeLookingForDocumentation(
      const std::string& line, const std::filesystem::path& current_file);

  std::size_t todo_count_;
  std::size_t fixme_count_;
  std::size_t file_count_;

  std::mutex print_lock_;
  std::mutex markdown_lock_;

  std::array<std::pair<const char*, std::size_t>, 2> keyword_pairs;

  static constexpr std::array<std::pair<const char*, CommentFormat>, 10>
      COMMENT_FORMATS{{
          {".c", CommentFormat::DoubleSlash},
          {".cpp", CommentFormat::DoubleSlash},
          {".h", CommentFormat::DoubleSlash},
          {".hpp", CommentFormat::DoubleSlash},
          {".js", CommentFormat::DoubleSlash},
          {".rs", CommentFormat::DoubleSlash},
          {".ts", CommentFormat::DoubleSlash},
          {".zig", CommentFormat::DoubleSlash},
          {".cs", CommentFormat::DoubleSlash},
          {".py", CommentFormat::PoundSign},
      }};
};

}  // namespace parser_info
#endif  // INCLUDE_PARSER_HPP_
