cmake_minimum_required(VERSION 3.19)
project(cpp_stub_cmake_smoke CXX)

set(CMAKE_CXX_STANDARD 11)

# Points at repository root (two levels up from this file).
add_subdirectory(../.. cpp-stub-build)

add_executable(cmake_smoke smoke.cpp)
target_link_libraries(cmake_smoke PRIVATE cpp-stub)

# Deliberately no cpp_stub_enable() call here: verifies that merely
# linking the interface target is enough on macOS.
