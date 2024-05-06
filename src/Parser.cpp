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
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <ios>
#include <iostream>
#include <mutex>
#include <optional>
#include <ostream>
#include <regex>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <vector>

namespace parser_info {
Parser::Parser(const bool&& verbose_printing)
    : verbose_printing_(verbose_printing),
      file_type_frequencies_{},
      file_count_(0),
      custom_regexes_{std::nullopt},
      thread_pool_capacity_{std::thread::hardware_concurrency() /
                            std::thread::hardware_concurrency()},
      active_threads_{0},
      keyword_pairs_{{
          {std::regex("\\bTODO(\\(\\w*\\))?"), 0, "TODO"},
          {std::regex("\\bFIXME(\\(\\w*\\))?"), 0, "FIXME"},
          {std::regex("\\bBUG(\\(\\w*\\))?"), 0, "BUG"},
          {std::regex("\\bHACK(\\(\\w*\\))?"), 0, "HACK"},
      }} {}

Parser::Parser(const bool&& verbose_printing,
               const std::vector<std::string>&& custom_regexes)
    : verbose_printing_(verbose_printing),
      file_type_frequencies_{},
      file_count_(0),
      custom_regexes_{std::make_optional(
          std::vector<
              std::tuple<std::regex, std::string_view, std::size_t>>{})},
      thread_pool_capacity_{std::thread::hardware_concurrency() /
                            std::thread::hardware_concurrency()},
      active_threads_{0},
      keyword_pairs_{{
          {std::regex("\\bTODO(\\(\\w*\\))?"), 0, "TODO"},
          {std::regex("\\bFIXME(\\(\\w*\\))?"), 0, "FIXME"},
          {std::regex("\\bBUG(\\(\\w*\\))?"), 0, "BUG"},
          {std::regex("\\bHACK(\\(\\w*\\))?"), 0, "HACK"},
      }} {
  custom_regexes_->reserve(custom_regexes.size());
  for (const std::string& regex : custom_regexes) {
    custom_regexes_->emplace_back(
        std::make_tuple<std::regex, std::string_view, std::size_t>(
            std::regex{regex}, std::string_view{regex}, 0));
  }
}

namespace {
inline std::optional<std::size_t> FindCommentPosition(
    const CommentFormat& comment_format, const std::string_view line,
    const std::filesystem::path& current_file) {
  switch (comment_format) {
    case CommentFormat::DoubleSlash:
      return line.find("//");
      break;
    case CommentFormat::PoundSign:
      return line.find("#");
      break;
  }

  /*
   * Only possible if new file type is added to the parser class but forgotten
   * to be checked in the above switch. This will allow our linter to warn of
   * non-exhaustive checks will appeasing the compiler of having a return on all
   * control paths without having a default case!
   */
  return std::nullopt;
}
}  // namespace

const std::optional<CommentFormat> Parser::IsValidFile(
    const std::filesystem::path& file) {
  std::filesystem::path extension(file.extension());
  for (const auto& [file_extension, classification] : Parser::COMMENT_FORMATS) {
    if (extension == file_extension) {
      file_type_frequencies_.try_emplace(file_extension, 0);
      file_type_frequencies_[file_extension]++;
      return classification;
    }
  }

  return std::nullopt;
}

void Parser::RecursivelyParseFiles(
    const std::filesystem::path& current_file) noexcept {
  if (std::filesystem::is_symlink(current_file) ||
      std::filesystem::is_directory(current_file)) {
    active_threads_.fetch_add(-1);
    return;
  }

  std::optional<CommentFormat> comment_format{this->IsValidFile(current_file)};

  if (!comment_format.has_value()) {
    active_threads_.fetch_add(-1);
    return;
  }

  std::ifstream file_stream(current_file);
  std::size_t line_count = 0;
  std::string line{};
  std::optional<std::size_t> comment_position{};
  std::size_t position{};
  std::string::iterator start{};
  file_count_++;

  while (std::getline(file_stream, line)) {
    line_count++;

    comment_position =
        FindCommentPosition(comment_format.value(), line, current_file);

    if (!comment_position.has_value()) {
      active_threads_.fetch_add(-1);
      return;
    } else if (comment_position.value() == std::string::npos) {
      continue;
    }

    start = line.begin() + comment_position.value();
    std::string sub_str(start, line.end());
    for (auto& [keyword_regex, keyword_count, keyword_literal] :
         keyword_pairs_) {
      if (std::regex_search(sub_str, keyword_regex)) {
        if (verbose_printing_) {
          std::lock_guard lock(print_lock_);
          std::cout << keyword_literal << " Found:" << std::endl
                    << "File: " << current_file << std::endl
                    << "Line Number: " << line_count << std::endl
                    << "Line: " << line << std::endl
                    << std::endl;
        }
        keyword_count++;
      }
    }

    if (custom_regexes_.has_value()) {
      for (auto& [regex, literal, count] : custom_regexes_.value()) {
        if (std::regex_search(sub_str, regex)) {
          if (verbose_printing_) {
            std::lock_guard lock(print_lock_);
            std::cout << "Regex " << literal << " Found:" << std::endl
                      << "File: " << current_file << std::endl
                      << "Line Number: " << line_count << std::endl
                      << "Line: " << line << std::endl
                      << std::endl;
          }
          count++;
        }
      }
    }
  }

  active_threads_.fetch_add(-1);
}

void Parser::ReportSummary() const {
  std::cout << std::endl
            << "------------------------------------ Summary ------------------"
               "-----------------"
            << std::endl;
  std::cout << std::endl
            << std::left << std::setw(19) << "File Extension" << std::left
            << "|" << std::left << std::setw(20) << "Files" << std::endl;
  std::cout << "---------------------------------------------------------------"
               "-----------------"
            << std::endl;
  for (const auto& [file_extension, frequency] : file_type_frequencies_) {
    std::cout << std::left << std::setw(19) << file_extension << "|"
              << std::setw(20) << frequency << std::endl;
    std::cout
        << "---------------------------------------------------------------"
           "-----------------"
        << std::endl;
  }
}

void Parser::ParseFiles(const std::filesystem::path& current_file) noexcept {
  std::cout << "Concurrent Threads Supported: " << thread_pool_capacity_
            << std::endl
            << std::endl;
  std::filesystem::recursive_directory_iterator directory_iterator(
      current_file);

  std::vector<std::jthread> pool{};
  pool.reserve(thread_pool_capacity_);

  std::ranges::for_each(directory_iterator,
                        [this, &pool](const std::filesystem::path& entry) {
                          if (pool.size() >= thread_pool_capacity_) {
                            pool.clear();
                          }

                          active_threads_.fetch_add(1);
                          pool.emplace_back(&Parser::RecursivelyParseFiles,
                                            this, std::ref(entry));
                        });

  pool.clear();

  std::cout << "Files Profiled: " << file_count_ << std::endl;
  for (const auto& [_, keyword_count, keyword_literal] : keyword_pairs_) {
    std::cout << keyword_literal << "s Found: " << keyword_count << std::endl;
  }  // TODO(not_a_real_todo) test
  if (custom_regexes_.has_value()) {
    std::cout
        << std::endl
        << "------------------------------------ Customs ------------------"
           "-----------------"
        << std::endl;
    for (const auto& [_, literal, count] : custom_regexes_.value()) {
      std::cout << "Amount of " << literal << " Found: " << count << std::endl;
    }
  }
  this->ReportSummary();
  std::cout << std::endl;
}

}  // namespace parser_info
