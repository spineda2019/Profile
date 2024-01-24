#include "Parser.hpp"

#include <filesystem>
#include <iostream>
#include <vector>

Parser::Parser(const std::vector<std::filesystem::path> files)
    : files_(std::move(files)) {}

void Parser::ListFiles() const {
  for (const auto& item : this->files_) {
    std::cout << item << std::endl;
  }
}

void Parser::ParseFiles() const {
  // TODO
}