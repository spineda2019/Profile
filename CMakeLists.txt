cmake_minimum_required(VERSION 3.18)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(Profile CXX)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# add the directories containing the header files to the search path
include_directories(src/include)

# collect all source files from the src directory
set(SOURCES "src/main.cpp" "src/directory_validator.cpp" "src/Parser.cpp")
# create the executable
add_executable(${PROJECT_NAME} ${SOURCES})
install(TARGETS Profile)
