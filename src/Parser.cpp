#include "Parser.hpp"

#include <filesystem>
#include <vector>
#include <iostream>

Parser::Parser(const std::vector<std::filesystem::path> files)
    : files_(std::move(files)) {}

Parser Parser::Create(const std::vector<std::filesystem::path> files) {
  Parser parser(std::move(files));
  return std::move(parser);
}

void Parser::ListFiles() const {
  for (const auto& item : this->files_) {
    std::cout << item << std::endl;
  }
}