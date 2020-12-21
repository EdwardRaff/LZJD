project(lzjd)
cmake_minimum_required(VERSION 2.8)
FIND_PACKAGE( Boost 1.50 COMPONENTS program_options system filesystem REQUIRED )
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )

set(CMAKE_CXX_FLAGS "-Wall -ggdb -std=c++11 -msse4.2")

add_executable(${PROJECT_NAME} ../src/LZJD.cpp ../src/MurmurHash3.cpp main.cpp)
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})

add_library(lzjd.shared SHARED ../src/LZJD.cpp ../src/MurmurHash3.cpp)
target_link_libraries(lzjd.shared)

add_library(lzjd.static STATIC ../src/LZJD.cpp ../src/MurmurHash3.cpp)

execute_process(COMMAND pg_config --includedir-server RESULT_VARIABLE PG_INCLUDESRV_RESULT OUTPUT_VARIABLE PG_INCLUDESRV_DIR)
execute_process(COMMAND pg_config --includedir RESULT_VARIABLE PG_INCLUDE_RESULT OUTPUT_VARIABLE PG_INCLUDE_DIR)

if(${PG_INCLUDESRV_RESULT} EQUAL 0)
  MESSAGE( STATUS "PG Server Include: ${PG_INCLUDESRV_DIR}" )
  MESSAGE( STATUS "PG Include: ${PG_INCLUDE_DIR}" )

  add_library(lzjd_psql SHARED ../src/pglzjd.c ../src/pg_lzjd_helper.cpp ../src/LZJD.cpp ../src/MurmurHash3.cpp)
  target_link_libraries(lzjd_psql ${Boost_LIBRARIES})
  target_include_directories(lzjd_psql PRIVATE ${PG_INCLUDESRV_DIR})
  #target_include_directories(lzjd_psql.so PRIVATE ${PG_INCLUDE_DIR})
  set_target_properties(lzjd_psql PROPERTIES PREFIX "")
  set_target_properties(lzjd_psql PROPERTIES OUTPUT_NAME "lzjd_psql")
endif()

set(CMAKE_BUILD_TYPE Release)

