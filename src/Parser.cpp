/*
 *  Parser.cpp - Implementation for file parsing and comment searching
 *  Copyright (C) 2024  Sebastian Pineda (spineda.wpi.alum@gmail.com)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program. If not, see <https://www.gnu.org/licenses/>
 */

#include "include/Parser.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
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
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

namespace parser_info {
Parser::Parser(const bool&& verbose_printing)
    : keyword_pairs_{{
          {std::regex("\\bTODO(\\(\\w*\\))?"), 0, "TODO"},
          {std::regex("\\bFIXME(\\(\\w*\\))?"), 0, "FIXME"},
          {std::regex("\\bBUG(\\(\\w*\\))?"), 0, "BUG"},
          {std::regex("\\bHACK(\\(\\w*\\))?"), 0, "HACK"},
      }},
      jobs_{},
      print_lock_{},
      job_lock_{},
      data_lock_{},
      file_type_frequencies_{},
      comment_formats_{
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
      },
      custom_regexes_{std::nullopt},
      thread_pool_{},
      job_semaphore_{0},
      file_count_{0},
      verbose_printing_{verbose_printing} {}

Parser::Parser(const bool&& verbose_printing,
               const std::vector<std::string>&& custom_regexes)
    : keyword_pairs_{{
          {std::regex("\\bTODO(\\(\\w*\\))?"), 0, "TODO"},
          {std::regex("\\bFIXME(\\(\\w*\\))?"), 0, "FIXME"},
          {std::regex("\\bBUG(\\(\\w*\\))?"), 0, "BUG"},
          {std::regex("\\bHACK(\\(\\w*\\))?"), 0, "HACK"},
      }},
      jobs_{},
      print_lock_{},
      job_lock_{},
      data_lock_{},
      file_type_frequencies_{},
      comment_formats_{
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
      },
      custom_regexes_{std::make_optional(
          std::vector<std::tuple<std::regex, std::string_view, std::size_t>>{
              custom_regexes.size()})},
      thread_pool_{},
      job_semaphore_{0},
      file_count_{0},
      verbose_printing_{verbose_printing} {
    for (const std::string& regex : custom_regexes) {
        custom_regexes_->emplace_back(
            std::make_tuple<std::regex, std::string_view, std::size_t>(
                std::regex{regex}, std::string_view{regex}, 0));
    }
}

namespace {
inline std::string_view::iterator FindCommentPosition(
    const CommentFormat& comment_format, const std::string_view line) {
    switch (comment_format) {
        case CommentFormat::DoubleSlash:
            return std::ranges::find_first_of(line, "//");
        case CommentFormat::PoundSign:
            return std::ranges::find_first_of(line, "#");
            break;
    }
}
}  // namespace

void Parser::ThreadWaitingRoom() {
    std::filesystem::path entry{};

    while (true) {
        job_semaphore_.acquire();

        if (std::unique_lock<std::mutex> lock{job_lock_}; !jobs_.empty())
            [[likely]] {
            entry.swap(jobs_.front());
            jobs_.pop();
        } else [[unlikely]] {
            job_semaphore_.release();
            return;
        }

        this->RecursivelyParseFiles(entry);
    }
}

const std::optional<CommentFormat> Parser::IsValidFile(
    const std::string&& extension) {
    if (comment_formats_.contains(extension)) {
        if (std::scoped_lock<std::mutex> lock{data_lock_};
            file_type_frequencies_.contains(extension)) {
            file_type_frequencies_[extension]++;
        } else {
            file_type_frequencies_.emplace(extension, 1);
        }

        return comment_formats_.at(extension);
    } else {
        return std::nullopt;
    }
}

void Parser::RecursivelyParseFiles(const std::filesystem::path& current_file) {
    std::optional<CommentFormat> comment_format{
        this->IsValidFile(current_file.extension().string())};

    if (!comment_format.has_value()) {
        return;
    } else {
        file_count_.fetch_add(1);
    }

    std::ifstream file_stream{current_file};
    std::size_t line_count{1};

    for (std::string line{}; std::getline(file_stream, line); ++line_count) {
        const std::string_view full_view{line};

        if (std::string_view sub_str{
                FindCommentPosition(comment_format.value(), full_view),
                full_view.cend()};
            !sub_str.empty()) {
            for (auto& [keyword_regex, keyword_count, keyword_literal] :
                 keyword_pairs_) {
                if (std::regex_search(sub_str.cbegin(), sub_str.cend(),
                                      keyword_regex)) {
                    if (verbose_printing_) {
                        std::scoped_lock<std::mutex> lock{print_lock_};
                        std::cout << keyword_literal << " Found:" << std::endl
                                  << "File: " << current_file << std::endl
                                  << "Line Number: " << line_count << std::endl
                                  << "Line: " << line << std::endl
                                  << std::endl;
                    }

                    keyword_count.fetch_add(1);
                }
            }

            if (custom_regexes_.has_value()) {
                for (auto& [regex, literal, count] : custom_regexes_.value()) {
                    if (std::regex_search(sub_str.cbegin(), sub_str.cend(),
                                          regex)) {
                        if (verbose_printing_) {
                            std::scoped_lock<std::mutex> lock{print_lock_};
                            std::cout
                                << "Regex " << literal << " Found:" << std::endl
                                << "File: " << current_file << std::endl
                                << "Line Number: " << line_count << std::endl
                                << "Line: " << line << std::endl
                                << std::endl;
                        }

                        std::scoped_lock<std::mutex> lock{data_lock_};
                        count++;
                    }
                }
            }
        }
    }
}

void Parser::ReportSummary() const {
    std::cout
        << std::endl
        << "------------------------------------ Summary ------------------"
           "-----------------"
        << std::endl;
    std::cout << std::endl
              << std::left << std::setw(19) << "File Extension" << std::left
              << "|" << std::left << std::setw(20) << "Files" << std::endl;
    std::cout
        << "---------------------------------------------------------------"
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
    const std::uint32_t thread_capacity = std::thread::hardware_concurrency();

    std::cout << "Concurrent Threads Supported: " << thread_capacity
              << std::endl
              << std::endl;

    for (std::uint32_t threads{0}; threads < thread_capacity; threads++) {
        thread_pool_.emplace_back(&Parser::ThreadWaitingRoom, this);
    }

    std::filesystem::recursive_directory_iterator directory_iterator(
        current_file);

    std::ranges::for_each(
        directory_iterator, [this](const std::filesystem::path& entry) {
            if (!(std::filesystem::is_symlink(entry) ||
                  std::filesystem::is_directory(entry))) [[likely]] {
                {
                    std::unique_lock<std::mutex> lock{job_lock_};
                    jobs_.emplace(std::move(entry));
                }
                job_semaphore_.release();
            }
        });

    std::ranges::for_each(thread_pool_, [this](std::jthread& t) {
        job_semaphore_.release();
        t.join();
    });

    std::cout << "Files Profiled: " << file_count_.load() << std::endl;
    for (const auto& [_, keyword_count, keyword_literal] : keyword_pairs_) {
        std::cout << keyword_literal << "s Found: " << keyword_count
                  << std::endl;
    }  // TODO(not_a_real_todo) test
    if (custom_regexes_.has_value()) {
        std::cout
            << std::endl
            << "------------------------------------ Customs ------------------"
               "-----------------"
            << std::endl;
        for (const auto& [_, literal, count] : custom_regexes_.value()) {
            std::cout << "Amount of " << literal << " Found: " << count
                      << std::endl;
        }
    }
    this->ReportSummary();
    std::cout << std::endl;
}

}  // namespace parser_info
