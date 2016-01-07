cmake_minimum_required(VERSION 2.8)

project(l2l1 CXX)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()
string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE)

message(STATUS "C++ compiler ID = ${CMAKE_CXX_COMPILER_ID}")
if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
  set(MY_CXX_FLAGS "-std=c++0x -Wall -Wextra -Wshadow -Weffc++ -pedantic -Werror")
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
  set(MY_CXX_FLAGS "-std=c++0x -Wall -Wextra -Wshadow -Weffc++ -pedantic -Werror")
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "Intel")
  set(MY_CXX_FLAGS "-std=c++0x -Wall -Wextra -Wshadow -pedantic -Werror")
else()
  message(WARNING "Unknown compiler!")
  set(MY_CXX_FLAGS "")
endif()

if(${BUILD_TYPE} STREQUAL "DEBUG")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${MY_CXX_FLAGS} -fno-inline")
  message("Debug flags: ${CMAKE_CXX_FLAGS_DEBUG}")
elseif(${BUILD_TYPE} STREQUAL "RELEASE")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${MY_CXX_FLAGS}")
  message("Release flags: ${CMAKE_CXX_FLAGS_RELEASE}")
else()
  message(FATAL_ERROR "Unknown build type: ${BUILD_TYPE}")
endif()

file(GLOB SRC_LIST "${PROJECT_SOURCE_DIR}/sources/*.cpp") # .cpp files
file(GLOB HDR_LIST "${PROJECT_SOURCE_DIR}/headers/*.hpp") # .hpp files
include_directories("${PROJECT_SOURCE_DIR}/headers")

add_executable(${PROJECT_NAME} ${SRC_LIST} ${HDR_LIST})

