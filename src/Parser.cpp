#include "Parser.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace parser_info {

Parser::Parser()
    : directory_threads_{},
      data_lock_{},
      todo_count_(0),
      fixme_count_(0),
      file_count_(0) {}

UnexpectedFileTypeException::UnexpectedFileTypeException(
    std::filesystem::path bad_file)
    : bad_file_(bad_file) {}

const char* UnexpectedFileTypeException::what() noexcept {
  std::stringstream error_message("FATAL: Unexpected filetype found: ");
  error_message << this->bad_file_;
  return error_message.str().c_str();
}

const bool Parser::IsValidFile(const std::filesystem::path& file,
                               CommentFormat& comment_format) const {
  std::filesystem::path extension(file.extension());

  if (std::find(this->double_slash_extensions_.begin(),
                this->double_slash_extensions_.end(),
                extension) != this->double_slash_extensions_.end()) {
    comment_format = CommentFormat::DoubleSlash;
    return true;
  } else if (std::find(this->pound_sign_extensions_.begin(),
                       this->pound_sign_extensions_.end(),
                       extension) != this->pound_sign_extensions_.end()) {
    comment_format = CommentFormat::PoundSign;
    return true;
  } else {
    comment_format = CommentFormat::None;
    return false;
  }
}

void Parser::RecursivelyParseFiles(const std::filesystem::path& current_file) {
  if (std::filesystem::is_symlink(current_file)) {
    return;
  }

  if (std::filesystem::is_directory(current_file)) {
    for (const auto& entry :
         std::filesystem::directory_iterator(current_file)) {
      this->directory_threads_.push_back(
          std::thread(&Parser::RecursivelyParseFiles, this, entry));
    }
  }

  std::fstream file_stream(current_file);
  std::size_t line_count = 0;
  std::string line{};
  CommentFormat comment_format{};
  std::size_t comment_position{};
  std::size_t todo_position{};
  std::size_t fixme_position{};

  if (!this->IsValidFile(current_file, comment_format)) {
    return;
  }

  {
    std::lock_guard lock(this->data_lock_);
    this->file_count_++;
  }

  while (std::getline(file_stream, line)) {
    line_count++;
    switch (comment_format) {
      case CommentFormat::DoubleSlash:
        comment_position = line.find("//");
        break;
      case CommentFormat::PoundSign:
        comment_position = line.find("#");
        break;
      default:  // Should be impossible, but let's be safe
        std::cerr << "Unexpected file type: " << current_file.extension()
                  << std::endl;
        UnexpectedFileTypeException file_exception(current_file);
        throw file_exception;
        break;
    }
    if (comment_position == std::string::npos) {
      continue;
    }

    todo_position = line.find("TODO", comment_position);
    fixme_position = line.find("FIXME", comment_position);
    if (todo_position != std::string::npos &&
        comment_position < todo_position) {
      std::cout << "TODO Found:" << std::endl
                << "File: " << current_file << std::endl
                << "Line Number: " << line_count << std::endl
                << "Line: " << line << std::endl
                << std::endl;
      {
        std::lock_guard lock(this->data_lock_);
        this->todo_count_++;
      }
    }

    if (fixme_position != std::string::npos &&
        comment_position < fixme_position) {
      std::cout << "FIXME Found:" << std::endl
                << "File: " << current_file << std::endl
                << "Line Number: " << line_count << std::endl
                << "Line: " << line << std::endl
                << std::endl;
      {
        std::lock_guard lock(this->data_lock_);
        this->todo_count_++;
      }
    }
  }
}

[[nodiscard]] int Parser::ParseFiles(
    const std::filesystem::path& current_file) {
  try {
    this->RecursivelyParseFiles(current_file);

    for (auto& thread : this->directory_threads_) {
      if (thread.joinable()) {
        thread.join();
      }
    }

    std::cout << "Files Profiled: " << this->file_count_ << std::endl;
    std::cout << "TODOs Found: " << this->todo_count_
              << std::endl;  // TODO test
    std::cout << "FIXMEs Found: " << this->fixme_count_ << std::endl
              << std::endl;

    return 0;
  } catch (const std::exception& e) {
    std::cout << "Unexpected Exception Thrown: " << e.what() << std::endl;
    std::cout << "Files Profiled: " << this->file_count_ << std::endl;
    std::cout << "TODOs Found: " << this->todo_count_
              << std::endl;  // TODO test
    std::cout << "FIXMEs Found: " << this->fixme_count_ << std::endl
              << std::endl;

    return Parser::FATAL_UNKNOWN_ERROR;
  }
}
}  // namespace parser_info