cmake_minimum_required(VERSION 3.18)
if(WIN32)
    set(CMAKE_TOOLCHAIN_FILE "C:\\Program Files\\vcpkg\\scripts\\buildsystems\\vcpkg.cmake")
endif(WIN32)
project(Profile)
# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# add the directories containing the header files to the search path
include_directories(include)

# collect all source files from the src directory
set(SOURCES "src/main.cpp" "src/directory_validator.cpp" "src/Parser.cpp")
# create the executable
add_executable(${PROJECT_NAME} ${SOURCES})

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_libraries(Profile pthread)
endif()
