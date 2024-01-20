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