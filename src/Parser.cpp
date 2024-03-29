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

#include "Parser.hpp"

#include <algorithm>
#include <execution>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
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
  std::string error_message_string = error_message.str();
  const char* what_message = error_message_string.c_str();
  return what_message;
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

std::size_t Parser::FindCommentPosition(
    const CommentFormat& comment_format, const std::string& line,
    const std::filesystem::path& current_file) {
  switch (comment_format) {
    case CommentFormat::DoubleSlash:
      return line.find("//");
      break;
    case CommentFormat::PoundSign:
      return line.find("#");
      break;
    default:  // Should be impossible, but let's be safe
      std::cerr << "Unexpected file type: " << current_file.extension()
                << std::endl;
      UnexpectedFileTypeException file_exception(current_file);
      throw file_exception;
      break;
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

    comment_position =
        Parser::FindCommentPosition(comment_format, line, current_file);

    if (comment_position == std::string::npos) {
      continue;
    }

    todo_position = line.find("TODO", comment_position);
    fixme_position = line.find("FIXME", comment_position);
    if (todo_position != std::string::npos) {
      std::lock_guard lock(this->print_lock_);
      std::cout << "TODO Found:" << std::endl
                << "File: " << current_file << std::endl
                << "Line Number: " << line_count << std::endl
                << "Line: " << line << std::endl
                << std::endl;
      this->todo_count_++;
    }

    if (fixme_position != std::string::npos) {
      std::lock_guard lock(this->print_lock_);
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

    return_code = Parser::SUCCESS;
  } catch (const UnexpectedFileTypeException& e) {
    std::cout << e.what() << std::endl;
  } catch (const std::exception& e) {
    std::cout << "Unexpected Exception Thrown: " << e.what() << std::endl;
    return_code = Parser::FATAL_UNEXPECTED_FILETYPE_ERROR;
  } catch (...) {
    std::cout << "UNKNOWN EXCEPTION CAUGHT" << std::endl;
    return_code = Parser::FATAL_UNKNOWN_ERROR;
  }

  std::cout << "Files Profiled: " << this->file_count_ << std::endl;
  std::cout << "TODOs Found: " << this->todo_count_
            << std::endl;  // TODO(not_a_real_todo) test
  std::cout << "FIXMEs Found: " << this->fixme_count_ << std::endl << std::endl;
  return return_code;
}

bool Parser::AreWeLookingForDocumentation(
    const std::string& line, const std::filesystem::path& current_file) {
  // TODO: Determine what we're looking for based on file
  return false;
}

void Parser::RecursivelyDocumentFiles(const std::filesystem::path& current_file,
                                      std::ofstream& output_markdown) {
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
        this->RecursivelyDocumentFiles(entry, output_markdown);
      }
    }
    std::for_each(
        std::execution::par_unseq, directories.begin(), directories.end(),
        [this, &output_markdown](const std::filesystem::path& directory) {
          this->RecursivelyDocumentFiles(directory, output_markdown);
        });
  }

  CommentFormat comment_format{};

  if (!this->IsValidFile(current_file, comment_format)) {
    return;
  }

  std::fstream file_stream(current_file);
  std::string line{};
  std::size_t comment_position{};
  std::unique_ptr<std::vector<std::stringstream>> file_info =
      std::make_unique<std::vector<std::stringstream>>();

  std::stringstream title{};
  title << "## Information About: " << current_file;
  file_info->push_back(std::move(title));

  bool looking = false;
  while (std::getline(file_stream, line)) {
    looking =
        looking || Parser::AreWeLookingForDocumentation(line, current_file);
    if (!looking) {
      continue;
    }

    comment_position =
        Parser::FindCommentPosition(comment_format, line, current_file);

    if (comment_position == std::string::npos) {
      continue;
    }
  }

  std::lock_guard<std::mutex> guard(this->markdown_lock_);
  for (const std::stringstream& item : *file_info) {
    output_markdown << item.str() << std::endl;
  }
}

[[nodiscard]] int Parser::DocumentFiles(
    const std::filesystem::path& root_folder) {
  std::filesystem::path document_path(
      std::filesystem::canonical(std::filesystem::absolute(".")));
  document_path.append("Profile.md");

  std::ofstream document_file(document_path);

  document_file << "# " << document_path.parent_path() << std::endl;

  std::uint8_t return_code{};

  try {
    this->RecursivelyDocumentFiles(root_folder, document_file);
    return_code = Parser::SUCCESS;
  } catch (const UnexpectedFileTypeException& e) {
    std::cerr << e.what() << std::endl;
    return_code = Parser::FATAL_UNEXPECTED_FILETYPE_ERROR;
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return_code = Parser::FATAL_UNKNOWN_ERROR;
  }

  document_file.close();
  return return_code;
}
}  // namespace parser_info
