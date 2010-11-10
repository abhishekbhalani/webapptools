cmake_minimum_required(VERSION 2.8)

include_directories("include")

file(GLOB WE_SOURCES "sources/*.cpp")
file(GLOB WE_HEADERS "include/*.h")

if(WIN32)
  set( WE_SOURCES ${WE_SOURCES} "sources/externals/shared_object_win32.cpp")
else()
  set( WE_SOURCES ${WE_SOURCES} "sources/externals/shared_object_posix.cpp")
endif()

add_library(webEngine STATIC ${WE_SOURCES} ${WE_HEADERS})

if(BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()
