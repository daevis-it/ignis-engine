cmake_minimum_required(VERSION 3.20)
project(IgnisEngine LANGUAGES C CXX)

# Decisione D5: C++20, non 23. std::format esiste su GCC 13 e su MSVC 2022+;
# del C++23 i due compilatori supportano sottoinsiemi diversi.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)          # niente estensioni GNU: portabilità verso MSVC

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Utile a clangd / CLion
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(cmake/Dependencies.cmake)

add_subdirectory(Ignis)
add_subdirectory(IgnisEditor)
add_subdirectory(Sandbox)
