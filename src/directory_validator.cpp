#include <directory_validator.hpp>
#ifdef _WIN32
#include <windows.h>
#elif __APPLE__
#include <sys/stat.h>
#elif __linux__
#include <sys/stat.h>
#endif

namespace directory_validation {
bool DirectoryExists(const char* directory_name) {
#ifdef _WIN32
  DWORD ftyp = GetFileAttributesA(directory_name);
  if (ftyp == INVALID_FILE_ATTRIBUTES) {
    return false;  // something is wrong with your path!
  }
  if (ftyp & FILE_ATTRIBUTE_DIRECTORY) {
    return true;  // this is a directory!
  }
  return false;  // this is not a directory!
#elif __APPLE__
  struct stat info;

  if (stat(directory_name, &info) != 0)
    return false;
  else if (info.st_mode & S_IFDIR)
    return true;
  else
    return false;
#elif __linux__
  struct stat info;

  if (stat(directory_name, &info) != 0)
    return false;
  else if (info.st_mode & S_IFDIR)
    return true;
  else
    return false;
#else
  return false;
#endif
}
}  // namespace directory_validation