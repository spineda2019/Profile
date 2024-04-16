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
#include <mutex>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <utility>

namespace parser_info {

enum class CommentFormat : std::uint8_t {
  DoubleSlash,
  PoundSign,
};

class Parser {
 public:
  explicit Parser(const bool&& verbose_printing);
  ~Parser() = default;

  void ParseFiles(const std::filesystem::path& current_file) noexcept;

 private:
  const std::optional<CommentFormat> IsValidFile(
      const std::filesystem::path& file);

  void ReportSummary() const;

  void RecursivelyParseFiles(
      const std::filesystem::path& current_file) noexcept;

  static std::optional<std::size_t> FindCommentPosition(
      const std::optional<CommentFormat>& comment_format,
      const std::string_view line, const std::filesystem::path& current_file);

 private:
  std::unordered_map<std::string_view, std::size_t> file_type_frequencies_;
  std::array<std::pair<std::string_view, std::size_t>, 4> keyword_pairs_;

  std::mutex print_lock_;
  std::mutex markdown_lock_;

  std::size_t file_count_;

  bool verbose_printing_;

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
