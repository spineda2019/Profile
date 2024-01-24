#include "Parser.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

Parser::Parser(const std::vector<std::filesystem::path> files)
    : files_(std::move(files)) {}

void Parser::ListFiles() const {
  for (const auto& item : this->files_) {
    std::cout << item << std::endl;
  }
}

void Parser::ParseFiles() const {
  std::fstream file_stream{};
  std::string line{};

  std::size_t line_count{};
  std::size_t todo_count = 0;
  std::size_t fixme_count = 0;

  std::size_t todo_position{};
  std::size_t fixme_position{};
  std::size_t comment_position{};
  for (const std::filesystem::path& file : this->files_) {
    line_count = 0;
    file_stream = std::fstream(file);

    while (std::getline(file_stream, line)) {
      line_count++;
      comment_position = line.find("//");
      if (comment_position == std::string::npos) {
        continue;
      }

      todo_position = line.find("TODO", comment_position);
      fixme_position = line.find("FIXME", comment_position);
      if (todo_position != std::string::npos &&
          comment_position < todo_position) {
        std::cout << "TODO Found:" << std::endl
                  << "File: " << file << std::endl
                  << "Line Number: " << line_count << std::endl
                  << "Line: " << line << std::endl
                  << std::endl;
        todo_count++;
      }

      if (fixme_position != std::string::npos &&
          comment_position < fixme_position) {
        std::cout << "FIXME Found:" << std::endl
                  << "File: " << file << std::endl
                  << "Line Number: " << line_count << std::endl
                  << "Line: " << line << std::endl
                  << std::endl;
        fixme_count++;
      }
    }
  }

  std::cout << "TODOs Found: " << todo_count << std::endl;  // TODO test
  std::cout << "FIXMEs Found: " << fixme_count << std::endl << std::endl;
}