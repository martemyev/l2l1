cmake_minimum_required(VERSION 2.8)

project(l2l1)

message(STATUS "C++ compiler ID = ${CMAKE_CXX_COMPILER_ID}")
if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
  set(MY_CXX_FLAGS "-std=c++0x -Wall -Wextra -Wshadow -Weffc++ -pedantic -Werror")
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
  set(MY_CXX_FLAGS "-std=c++0x -Wall -Wextra -Wshadow -Weffc++ -pedantic -Werror")
else()
  set(MY_CXX_FLAGS "-std=c++0x -Wall -pedantic -Werror")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MY_CXX_FLAGS}")

file(GLOB SRC_LIST "${PROJECT_SOURCE_DIR}/sources/*.cpp") # .cpp files
file(GLOB HDR_LIST "${PROJECT_SOURCE_DIR}/headers/*.hpp") # .hpp files
include_directories("${PROJECT_SOURCE_DIR}/headers")

add_executable(${PROJECT_NAME} ${SRC_LIST} ${HDR_LIST})

