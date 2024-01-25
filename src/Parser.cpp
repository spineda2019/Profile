#include "Parser.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace parser_info {

Parser::Parser()
    : file_stream_{},
      line_{},
      line_count_{},
      todo_count_(0),
      fixme_count_(0),
      file_count_(0),
      todo_position_{},
      fixme_position_{},
      comment_position_{},
      comment_format_{} {}

const bool Parser::IsValidFile(const std::filesystem::path& file) {
  std::filesystem::path extension(file.extension());

  if (std::find(this->double_slash_extensions_.begin(),
                this->double_slash_extensions_.end(),
                extension) != this->double_slash_extensions_.end()) {
    this->comment_format_ = CommentFormat::DoubleSlash;
    return true;
  } else if (std::find(this->pound_sign_extensions_.begin(),
                       this->pound_sign_extensions_.end(),
                       extension) != this->pound_sign_extensions_.end()) {
    this->comment_format_ = CommentFormat::PoundSign;
    return true;
  } else {
    this->comment_format_ = CommentFormat::None;
    return false;
  }
}

[[nodiscard]] int Parser::RecursivelyParseFiles(
    const std::filesystem::path& current_file) {
  if (std::filesystem::is_directory(current_file)) {
    for (const auto& entry :
         std::filesystem::directory_iterator(current_file)) {
      int result = this->RecursivelyParseFiles(entry);
      if (result == -1) {
        return -1;
      } else if (result == 1) {
        continue;
      }
    }
  }

  if (!this->IsValidFile(current_file)) {
    return 1;
  }

  this->file_count_++;
  this->line_count_ = 0;
  this->file_stream_ = std::fstream(current_file);

  while (std::getline(this->file_stream_, this->line_)) {
    this->line_count_++;
    switch (this->comment_format_) {
      case CommentFormat::DoubleSlash:
        this->comment_position_ = this->line_.find("//");
        break;
      case CommentFormat::PoundSign:
        this->comment_position_ = this->line_.find("#");
        break;
      default:  // Should be impossible, but let's be safe
        std::cerr << "Unexpected file type: " << current_file.extension()
                  << std::endl;
        return -1;
        break;
    }
    if (this->comment_position_ == std::string::npos) {
      continue;
    }

    this->todo_position_ = this->line_.find("TODO", this->comment_position_);
    this->fixme_position_ = this->line_.find("FIXME", this->comment_position_);
    if (this->todo_position_ != std::string::npos &&
        this->comment_position_ < this->todo_position_) {
      std::cout << "TODO Found:" << std::endl
                << "File: " << current_file << std::endl
                << "Line Number: " << this->line_count_ << std::endl
                << "Line: " << this->line_ << std::endl
                << std::endl;
      this->todo_count_++;
    }

    if (this->fixme_position_ != std::string::npos &&
        this->comment_position_ < this->fixme_position_) {
      std::cout << "FIXME Found:" << std::endl
                << "File: " << current_file << std::endl
                << "Line Number: " << this->line_count_ << std::endl
                << "Line: " << this->line_ << std::endl
                << std::endl;
      this->todo_count_++;
    }
  }

  return 0;
}

[[nodiscard]] int Parser::ParseFiles(
    const std::filesystem::path& current_file) {
  int result = this->RecursivelyParseFiles(current_file);
  if (result == -1) {
    return -1;
  }
  std::cout << "Files Profiled: " << this->file_count_ << std::endl;
  std::cout << "TODOs Found: " << this->todo_count_ << std::endl;  // TODO test
  std::cout << "FIXMEs Found: " << this->fixme_count_ << std::endl << std::endl;

  return 0;
}
}  // namespace parser_info