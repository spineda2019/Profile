#include "Parser.hpp"

#include <algorithm>
#include <execution>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace parser_info {

Parser::Parser() : todo_count_(0), fixme_count_(0), file_count_(0) {}

UnexpectedFileTypeException::UnexpectedFileTypeException(
    std::filesystem::path bad_file)
    : bad_file_(bad_file) {}

const char* UnexpectedFileTypeException::what() const noexcept {
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
    std::vector<std::filesystem::path> directories{};
    for (const auto& entry :
         std::filesystem::directory_iterator(current_file)) {
      if (std::filesystem::is_directory(entry)) {
        directories.push_back(std::move(entry));
      } else {
        this->RecursivelyParseFiles(entry);
      }
    }
    std::for_each(std::execution::par_unseq, directories.begin(),
                  directories.end(),
                  [this](const std::filesystem::path& directory) {
                    this->RecursivelyParseFiles(directory);
                  });
  }

  CommentFormat comment_format{};

  if (!this->IsValidFile(current_file, comment_format)) {
    return;
  }

  std::fstream file_stream(current_file);
  std::size_t line_count = 0;
  std::string line{};
  std::size_t comment_position{};
  std::size_t todo_position{};
  std::size_t fixme_position{};
  this->file_count_++;

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
    if (todo_position != std::string::npos) {
      std::cout << "TODO Found:" << std::endl
                << "File: " << current_file << std::endl
                << "Line Number: " << line_count << std::endl
                << "Line: " << line << std::endl
                << std::endl;
      this->todo_count_++;
    }

    if (fixme_position != std::string::npos) {
      std::cout << "FIXME Found:" << std::endl
                << "File: " << current_file << std::endl
                << "Line Number: " << line_count << std::endl
                << "Line: " << line << std::endl
                << std::endl;
      this->fixme_count_++;
    }
  }
}

[[nodiscard]] int Parser::ParseFiles(
    const std::filesystem::path& current_file) {
  std::uint8_t return_code{};
  try {
    this->RecursivelyParseFiles(current_file);

    return_code = 0;
  } catch (const UnexpectedFileTypeException& e) {
    std::cout << e.what() << std::endl;
  } catch (const std::exception& e) {
    std::cout << "Unexpected Exception Thrown: " << e.what() << std::endl;
    return_code = Parser::FATAL_UNKNOWN_ERROR;
  } catch (...) {
    std::cout << "UNKNOWN EXCEPTION CAUGHT" << std::endl;
    return_code = Parser::FATAL_UNKNOWN_ERROR;
  }

  std::cout << "Files Profiled: " << this->file_count_ << std::endl;
  std::cout << "TODOs Found: " << this->todo_count_ << std::endl;  // TODO test
  std::cout << "FIXMEs Found: " << this->fixme_count_ << std::endl << std::endl;
  return return_code;
}
}  // namespace parser_info