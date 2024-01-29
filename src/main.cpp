#include <Parser.hpp>
#include <filesystem>
#include <iostream>

#include "directory_validator.hpp"

int main(int argc, char** argv) {
  std::filesystem::path directory{};

  if (argc < 2) {
    directory = std::filesystem::absolute(".");
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

  return parser.ParseFiles(std::move(directory));
}