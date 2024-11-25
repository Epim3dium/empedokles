cmake_minimum_required(VERSION 3.12)

project(empedokles
  VERSION 1.0
  LANGUAGES CXX
)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_C_FLAGS_DEBUG "-g")
set(CMAKE_STRIP "")

set(CMAKE_CXX_STANDARD 20)

if(POLICY CMP0079 )
  cmake_policy(SET CMP0079 NEW)
endif()

add_subdirectory(demo)
add_subdirectory(tests)
add_subdirectory(src)

include(cmake/Install.cmake)
