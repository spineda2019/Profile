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

#include "include/Parser.hpp"
#include "include/directory_validator.hpp"

int main(int argc, char** argv) {
  argparse::ArgumentParser argument_parser("main");
  argument_parser.add_argument("-h", "--help")
      .help("Display This Message And Exit")
      .default_value(false)
      .implicit_value(true);

  argument_parser.add_argument("-V", "--version")
      .help("Display Program Version")
      .default_value(false)
      .implicit_value(true);

  try {
    argument_parser.parse_args(argc, argv);
  } catch (const std::exception& err) {
    std::cerr << err.what() << std::endl;
    std::cerr << argument_parser;
    std::exit(1);
  }

  if (argument_parser.get<bool>("-h") || argument_parser.get<bool>("--help")) {
    std::cout << argument_parser;
    return 0;
  } else if (argument_parser.get<bool>("-v") ||
             argument_parser.get<bool>("--version")) {
    std::cout << "Profile 0.0.1" << std::endl;
    return 0;
  }

  std::filesystem::path directory{};

  if (argc != 2) {
    directory = std::filesystem::canonical(std::filesystem::absolute("."));
  } else {
    directory = std::filesystem::absolute(argv[1]);
  }

  std::cout << "Profiling Directory " << directory << std::endl << std::endl;

  if (!directory_validation::DirectoryExists(directory)) {
    std::cerr << "FATAL: Directory " << directory << " does not exist!"
              << std::endl;
    return -1;
  }

  parser_info::Parser parser{};

  int parse_result = parser.ParseFiles(directory);
  if (parse_result) {
    return parse_result;
  }

  //   int markdown_result = parser.DocumentFiles(directory);
  //   if (markdown_result) {
  //     return markdown_result;
  //   }

  return 0;
}
