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

#include <directory_validator.hpp>
#include <filesystem>
#include <vector>

namespace directory_validation {
bool DirectoryExists(const std::filesystem::path& directory_name) {
  if (!std::filesystem::exists(directory_name)) {
    return false;
  } else {
    return true;
  }
}

void GetFilesToProfile(const std::filesystem::path& directory,
                       std::vector<std::filesystem::path>& found_files) {
  if (std::filesystem::is_directory(directory)) {
    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
      GetFilesToProfile(entry.path(), found_files);
    }
  } else if (std::filesystem::is_regular_file(directory)) {
    found_files.push_back(directory);
  }
}
}  // namespace directory_validation
