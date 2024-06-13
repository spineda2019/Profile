/*
 *  main.cpp - CLI setup and pass along to parser
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

#include <argparse/argparse.hpp>
#include <array>
#include <filesystem>
#include <iomanip>
#include <ios>
#include <iostream>
#include <span>
#include <string_view>
#include <utility>
#include <vector>

#include "include/Parser.hpp"
#include "include/directory_validator.hpp"

constexpr bool NoEmptyRegexes(const std::span<std::string> regexes) {
  for (const std::string_view regex : regexes) {
    if (regex.size() == 0) {
      return false;
    }
  }
  return true;
}

constexpr std::array<std::pair<std::string_view, std::string_view>, 10>
    supported_languages{{
        {"C", "//"},
        {"C++", "//"},
        {"C Header Files", "//"},
        {"C++ Header Files", "//"},
        {"JavaScript", "//"},
        {"TypeScript", "//"},
        {"Rust", "//"},
        {"Zig", "//"},
        {"C#", "//"},
        {"Python", "#"},
    }};

constexpr std::size_t max_column_width{18};

constexpr const char* version{"1.0.1"};

int main(int argc, char** argv) {
  argparse::ArgumentParser argument_parser("Profile", version,
                                           argparse::default_arguments::none);

  argument_parser.add_argument("--directory", "-d")
      .help("Directory to Profile");

  argument_parser.add_argument("-c", "--custom")
      .default_value(std::vector<std::string>{})
      .append()
      .help("Custom Regexes");

  argument_parser.add_argument("--log", "-l")
      .help("Log Found Comment to Stdout")
      .flag();

  argument_parser.add_argument("-h", "--help")
      .help("Display This Message And Exit")
      .flag();

  argument_parser.add_argument("-v", "--version")
      .help("Display Program Version")
      .flag();

  argument_parser.add_argument("-a", "--list-all")
      .help("List Recognized Filetypes")
      .flag();

  try {
    argument_parser.parse_args(argc, argv);
  } catch (const std::exception& err) {
    std::cerr << err.what() << std::endl;
    std::cerr << argument_parser;
    std::exit(1);
  }

  if (argument_parser.get<bool>("-h")) {
    std::cout << argument_parser;
    return 0;
  } else if (argument_parser.get<bool>("-v")) {
    std::cout << "Profile " << version << std::endl;
    std::cout
        << "Copyright (c) 2024 Sebastian Pineda (spineda.wpi.alum@gmail.com)"
        << std::endl;
    std::cout
        << "This program is free software; you may redistribute it under the "
           "terms of the\nGNU General Public License version 2 or (at your "
           "option) any later version. This\nprogram has absolutely no "
           "warranty."
        << std::endl
        << std::endl;
    return 0;
  } else if (argument_parser.get<bool>("-a")) {
    std::cout << std::endl
              << "Supported Languages (Contact spineda.wpi.alum@gmail.com or "
                 "submit a github issue for suggestions):"
              << std::endl
              << std::endl
              << "-------------------------------" << std::endl
              << std::left << std::setw(max_column_width) << "Language"
              << std::left << "|" << std::left << std::setw(max_column_width)
              << "Comment Type" << std::endl
              << "-------------------------------" << std::endl;
    for (const auto [file_type, comment_type] : supported_languages) {
      std::cout << std::left << std::setw(max_column_width) << file_type
                << std::left << "|" << std::string{max_column_width / 2}
                << std::setw(max_column_width) << comment_type << std::endl;
    }
    return 0;
  }

  std::optional<std::string> dir_str = argument_parser.present("-d");
  std::filesystem::path directory(std::filesystem::canonical(
      std::filesystem::absolute(dir_str.value_or("."))));

  if (!directory_validation::DirectoryExists(directory)) {
    std::cerr << "FATAL: Directory " << directory << " does not exist!"
              << std::endl;
    return -1;
  }

  std::cout << "Profiling Directory " << directory << std::endl << std::endl;

  if (std::vector<std::string> regexes{
          argument_parser.get<std::vector<std::string>>("-c")};
      regexes.size() != 0 && NoEmptyRegexes(regexes)) {
    parser_info::Parser parser{argument_parser.get<bool>("-l"),
                               std::move(regexes)};
    parser.ParseFiles(directory);
  } else {
    parser_info::Parser parser{argument_parser.get<bool>("-l")};
    parser.ParseFiles(directory);
  }

  return 0;
}
