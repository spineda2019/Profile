#include <filesystem>
#include <iostream>

#include "Parser.hpp"
#include "directory_validator.hpp"

int main(int argc, char** argv) {
  std::filesystem::path directory{};

  if (argc < 2) {
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

  int markdown_result = parser.DocumentFiles(directory);
  if (markdown_result) {
    return markdown_result;
  }

  return 0;
}
