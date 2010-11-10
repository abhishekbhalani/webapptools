cmake_minimum_required(VERSION 2.8)
project (watools)

include(../options.cmake)

include_directories(common)
include_directories(../webEngine/include)

if(${BUILD_DOMSHELL})
  add_definitions(-DV8_DOMSHELL)
else()
  add_subdirectory(crawler)
  add_subdirectory(scanner)
  add_subdirectory(reporter)
  add_subdirectory(connector)
endif()

add_subdirectory(../webEngine webEngine)
add_subdirectory(plugins)
add_subdirectory(../dom dom)
