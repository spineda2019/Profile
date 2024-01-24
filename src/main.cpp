#include <Parser.hpp>
#include <filesystem>
#include <iostream>

#include "directory_validator.hpp"

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "USE: Profile <DIRECTORY>" << std::endl;
    return -1;
  }

  std::filesystem::path directory(argv[1]);

  if (!directory_validation::DirectoryExists(directory)) {
    std::cerr << "FATAL: Directory " << directory << " does not exist!"
              << std::endl;
    return -1;
  }

  std::cout << "Profiling Directory " << directory << std::endl << std::endl;

  parser_info::Parser parser{};

  return parser.ParseFiles(std::move(directory));
}