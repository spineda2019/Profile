cmake_minimum_required(VERSION 3.18)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(Profile CXX)

# fetch latest argparse
include(FetchContent)
FetchContent_Declare(
    argparse
    GIT_REPOSITORY https://github.com/p-ranav/argparse.git
)
FetchContent_MakeAvailable(argparse)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# add the directories containing the header files to the search path
include_directories(src/include)

# collect all source files from the src directory
set(SOURCES "src/main.cpp" "src/directory_validator.cpp" "src/Parser.cpp")
# create the executable
add_executable(${PROJECT_NAME} ${SOURCES})

# link against argparse
target_link_libraries(${PROJECT_NAME} argparse)
install(TARGETS Profile)
