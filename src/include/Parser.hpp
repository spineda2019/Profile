/*
 *  Parser.hpp - Class structure for file parsing and comment searching
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

#ifndef INCLUDE_PARSER_HPP_
#define INCLUDE_PARSER_HPP_

#include <array>
#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <mutex>
#include <optional>
#include <queue>
#include <regex>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

namespace parser_info {

enum class CommentFormat : std::uint8_t {
  DoubleSlash,
  PoundSign,
};

class Parser {
 public:
  explicit Parser(const bool&& verbose_printing);
  explicit Parser(const bool&& verbose_printing,
                  const std::vector<std::string>&& custom_regexes);
  ~Parser() = default;

  void ParseFiles(const std::filesystem::path& current_file) noexcept;

 private:
  const std::optional<CommentFormat> IsValidFile(const std::string_view file);

  void ReportSummary() const;

  void RecursivelyParseFiles(const std::filesystem::path& current_file);

  void ThreadWaitingRoom();

 private:
  std::array<std::tuple<std::regex, std::atomic<std::size_t>, std::string_view>,
             4>
      keyword_pairs_;
  std::queue<
      std::pair<std::filesystem::path,
                std::function<void(Parser&, const std::filesystem::path&)>>>
      jobs_;
  std::mutex print_lock_;
  std::mutex job_lock_;
  std::mutex data_lock_;
  std::unordered_map<std::string, std::size_t> file_type_frequencies_;
  const std::unordered_map<std::string_view, CommentFormat> comment_formats_;
  std::condition_variable job_condition_;
  std::optional<
      std::vector<std::tuple<std::regex, std::string_view, std::size_t>>>
      custom_regexes_;
  std::vector<std::jthread> thread_pool_;
  std::atomic<std::size_t> file_count_;
  std::atomic<bool> terminate_jobs_;
  const bool verbose_printing_;
};

}  // namespace parser_info
#endif  // INCLUDE_PARSER_HPP_
