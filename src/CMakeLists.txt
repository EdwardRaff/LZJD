project(lzjd CXX)
cmake_minimum_required(VERSION 2.8)
FIND_PACKAGE( Boost 1.50 COMPONENTS program_options system filesystem REQUIRED )
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )

set(CMAKE_CXX_FLAGS "-Wall -ggdb -std=c++11 -msse4.2")

add_executable(${PROJECT_NAME} ../src/LZJD.cpp ../src/MurmurHash3.cpp main.cpp)
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})

add_library(lzjd.shared SHARED ../src/LZJD.cpp ../src/MurmurHash3.cpp)
target_link_libraries(lzjd.shared)

add_library(lzjd.static STATIC ../src/LZJD.cpp ../src/MurmurHash3.cpp)

set(CMAKE_BUILD_TYPE Release)

