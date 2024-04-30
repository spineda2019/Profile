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

#include <argparse/argparse.hpp>
#include <filesystem>
#include <iostream>
#include <span>
#include <string_view>
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

constexpr const char* version{"0.1.1"};

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
    std::cout << "This software is licensed under the MIT license. You should "
                 "have received it \nwith this copy of the software. If you "
                 "didn't, double check with your provider."
              << std::endl
              << std::endl;
    return 0;
  }

  std::optional<std::string> dir_str = argument_parser.present("-d");
  std::filesystem::path directory(std::filesystem::canonical(
      std::filesystem::absolute(dir_str.value_or("."))));

  std::cout << "Profiling Directory " << directory << std::endl << std::endl;

  if (!directory_validation::DirectoryExists(directory)) {
    std::cerr << "FATAL: Directory " << directory << " does not exist!"
              << std::endl;
    return -1;
  }

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
