cmake_minimum_required(VERSION 3.15)

project(CephableSample VERSION 1.0 LANGUAGES CXX)

# Use vcpkg for dependency management
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add executable
add_executable(CephableSample CephableSample.cpp)

# Find required packages
find_package(CPPREST REQUIRED CONFIG)
find_package(uwebsocket REQUIRED CONFIG)

# Link libraries to the target
target_link_libraries(CephableSample PRIVATE cpprest uwebsocket)
