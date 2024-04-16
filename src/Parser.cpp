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

#include "include/Parser.hpp"

#include <bits/fs_dir.h>

#include <algorithm>
#include <execution>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>

namespace parser_info {
Parser::Parser(const bool&& verbose_printing)
    : verbose_printing_(verbose_printing),
      file_count_(0),
      keyword_pairs_{{
          {"TODO", 0},
          {"FIXME", 0},
          {"BUG", 0},
          {"HACK", 0},
      }} {}

const std::optional<CommentFormat> Parser::IsValidFile(
    const std::filesystem::path& file) const {
  std::filesystem::path extension(file.extension());
  for (const auto& [file_extension, classification] : Parser::COMMENT_FORMATS) {
    if (extension == file_extension) {
      return classification;
    }
  }

  return std::nullopt;
}

std::optional<std::size_t> Parser::FindCommentPosition(
    const std::optional<CommentFormat>& comment_format,
    const std::string_view line, const std::filesystem::path& current_file) {
  if (!comment_format.has_value()) {
    std::cerr << "Unexpected file type: " << current_file.extension()
              << std::endl;
    return std::nullopt;
  } else {
    switch (comment_format.value()) {
      case CommentFormat::DoubleSlash:
        return line.find("//");
        break;
      case CommentFormat::PoundSign:
        return line.find("#");
        break;
    }
  }
}

void Parser::RecursivelyParseFiles(
    const std::filesystem::path& current_file) noexcept {
  if (std::filesystem::is_symlink(current_file)) {
    return;
  }

  if (std::filesystem::is_directory(current_file)) {
    std::filesystem::directory_iterator directory_iterator(current_file);
    std::for_each(std::execution::par_unseq,
                  std::filesystem::begin(directory_iterator),
                  std::filesystem::end(directory_iterator),
                  [this](const std::filesystem::path& entry) {
                    this->RecursivelyParseFiles(entry);
                  });
  }

  std::optional<CommentFormat> comment_format{this->IsValidFile(current_file)};

  if (!comment_format.has_value()) {
    return;
  }

  std::ifstream file_stream(current_file);
  std::size_t line_count = 0;
  std::string line{};
  std::optional<std::size_t> comment_position{};
  std::size_t position{};
  this->file_count_++;

  while (std::getline(file_stream, line)) {
    line_count++;

    comment_position =
        Parser::FindCommentPosition(comment_format, line, current_file);

    if (!comment_position.has_value()) {
      break;
    }

    if (comment_position.value() == std::string::npos) {
      continue;
    }

    if (this->verbose_printing_) {
      for (auto& [keyword, keyword_count] : this->keyword_pairs_) {
        position = line.find(keyword, comment_position.value());
        if (position != std::string::npos) {
          std::lock_guard lock(this->print_lock_);
          std::cout << keyword << " Found:" << std::endl
                    << "File: " << current_file << std::endl
                    << "Line Number: " << line_count << std::endl
                    << "Line: " << line << std::endl
                    << std::endl;
          keyword_count++;
        }
      }
    } else {
      for (auto& [keyword, keyword_count] : this->keyword_pairs_) {
        position = line.find(keyword, comment_position.value());
        if (position != std::string::npos) {
          keyword_count++;
        }
      }
    }
  }
}

void Parser::ParseFiles(const std::filesystem::path& current_file) noexcept {
  this->RecursivelyParseFiles(current_file);

  std::cout << "Files Profiled: " << this->file_count_ << std::endl;
  for (const auto& [keyword, keyword_count] : this->keyword_pairs_) {
    std::cout << keyword << "s Found: " << keyword_count << std::endl;
  }  // TODO(not_a_real_todo) test
  std::cout << std::endl;
}

}  // namespace parser_info
