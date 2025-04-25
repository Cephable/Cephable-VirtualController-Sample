cmake_minimum_required(VERSION 3.10)

# Project name
project(CephableClient)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add executable
add_executable(CephableClient CephableClient.cpp)

# Find and link required libraries
find_package(CURL REQUIRED)
find_package(Boost REQUIRED COMPONENTS system thread)
find_package(microsoft-signalr REQUIRED)
find_package(cpprestsdk REQUIRED)

# Include directories
include_directories(${CURL_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIRS})

# Link libraries
target_link_libraries(CephableClient ${CURL_LIBRARIES} ${Boost_LIBRARIES})
target_link_libraries(CephableClient PRIVATE cpprestsdk::cpprest)
link_libraries(microsoft-signalr::microsoft-signalr)

# Add SignalR client library
# Assuming SignalR client library is installed in /usr/local/include and /usr/local/lib
include_directories(/usr/local/include)

# Output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)