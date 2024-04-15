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

#include <algorithm>
#include <execution>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace parser_info {
namespace {
constexpr std::array<const char*, 2> keywords{"TODO", "FIXME"};
}

Parser::Parser()
    : file_count_(0),
      keyword_pairs_{{
          {"TODO", 0},
          {"FIXME", 0},
      }} {}

const std::optional<CommentFormat> Parser::IsValidFile(
    const std::filesystem::path& file) const {
  std::filesystem::path extension(file.extension());
  for (const auto [file_extension, classification] : Parser::COMMENT_FORMATS) {
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

void Parser::RecursivelyParseFiles(const std::filesystem::path& current_file) {
  if (std::filesystem::is_symlink(current_file)) {
    return;
  }

  if (std::filesystem::is_directory(current_file)) {
    std::vector<std::filesystem::path> directories{};
    for (const auto& entry :
         std::filesystem::directory_iterator(current_file)) {
      if (std::filesystem::is_directory(entry)) {
        directories.push_back(std::move(entry));
      } else {
        this->RecursivelyParseFiles(entry);
      }
    }
    std::for_each(std::execution::par_unseq, directories.begin(),
                  directories.end(),
                  [this](const std::filesystem::path& directory) {
                    this->RecursivelyParseFiles(directory);
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

    for (auto [keyword, keyword_count] : this->keyword_pairs_) {
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
  }
}

[[nodiscard]] int Parser::ParseFiles(
    const std::filesystem::path& current_file) noexcept {
  this->RecursivelyParseFiles(current_file);

  std::cout << "Files Profiled: " << this->file_count_ << std::endl;
  for (const auto [keyword, keyword_count] : this->keyword_pairs_) {
    std::cout << keyword << "s Found: " << keyword_count << std::endl;
  }  // TODO(not_a_real_todo) test
  std::cout << std::endl;
  return 0;
}

bool Parser::AreWeLookingForDocumentation(
    const std::string& line, const std::filesystem::path& current_file) {
  /* TODO: Determine what we're looking for based on file */
  return false;
}

void Parser::RecursivelyDocumentFiles(const std::filesystem::path& current_file,
                                      std::ofstream& output_markdown) {
  if (std::filesystem::is_symlink(current_file)) {
    return;
  }

  if (std::filesystem::is_directory(current_file)) {
    std::vector<std::filesystem::path> directories{};
    for (const auto& entry :
         std::filesystem::directory_iterator(current_file)) {
      if (std::filesystem::is_directory(entry)) {
        directories.push_back(std::move(entry));
      } else {
        this->RecursivelyDocumentFiles(entry, output_markdown);
      }
    }
    std::for_each(
        std::execution::par_unseq, directories.begin(), directories.end(),
        [this, &output_markdown](const std::filesystem::path& directory) {
          this->RecursivelyDocumentFiles(directory, output_markdown);
        });
  }

  std::optional<CommentFormat> comment_format{this->IsValidFile(current_file)};
  if (!comment_format.has_value()) {
    return;
  }

  std::fstream file_stream(current_file);
  std::string line{};
  std::optional<std::size_t> comment_position{};
  std::unique_ptr<std::vector<std::stringstream>> file_info =
      std::make_unique<std::vector<std::stringstream>>();

  std::stringstream title{};
  title << "## Information About: " << current_file;
  file_info->push_back(std::move(title));

  bool looking = false;
  while (std::getline(file_stream, line)) {
    looking =
        looking || Parser::AreWeLookingForDocumentation(line, current_file);
    if (!looking) {
      continue;
    }

    comment_position =
        Parser::FindCommentPosition(comment_format, line, current_file);

    if (!comment_position.has_value()) {
      break;
    }

    if (comment_position.value() == std::string::npos) {
      continue;
    }
  }

  std::lock_guard<std::mutex> guard(this->markdown_lock_);
  for (const std::stringstream& item : *file_info) {
    output_markdown << item.str() << std::endl;
  }
}

[[nodiscard]] int Parser::DocumentFiles(
    const std::filesystem::path& root_folder) {
  std::filesystem::path document_path(
      std::filesystem::canonical(std::filesystem::absolute(".")));
  document_path.append("Profile.md");

  std::ofstream document_file(document_path);

  document_file << "# " << document_path.parent_path() << std::endl;

  this->RecursivelyDocumentFiles(root_folder, document_file);

  document_file.close();
  return 0;
}
}  // namespace parser_info
