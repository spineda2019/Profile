#ifndef PROFILE_INCLUDE_DIRECTORY_VALIDATOR_HPP
#define PROFILE_INCLUDE_DIRECTORY_VALIDATOR_HPP

#include <array>
#include <filesystem>
#include <vector>

namespace directory_validation {
bool DirectoryExists(const std::filesystem::path& directory_name);

void GetFilesToProfile(const std::filesystem::path& directory,
                       std::vector<std::filesystem::path>& found_files);
}  // namespace directory_validation
#endif  // PROFILE_INCLUDE_DIRECTORY_VALIDATOR_HPP