/*
 *  directory_validator.hpp - function declarations for passed arg validation
 *  Copyright (C) 2024  Sebastian Pineda (spineda.wpi.alum@gmail.com)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program. If not, see <https://www.gnu.org/licenses/>
 */

#ifndef SRC_INCLUDE_DIRECTORY_VALIDATOR_HPP_
#define SRC_INCLUDE_DIRECTORY_VALIDATOR_HPP_

#include <filesystem>

namespace directory_validation {
bool DirectoryExists(const std::filesystem::path& directory_name);
}  // namespace directory_validation
#endif  // SRC_INCLUDE_DIRECTORY_VALIDATOR_HPP_
