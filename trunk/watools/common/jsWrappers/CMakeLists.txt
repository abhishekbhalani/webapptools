cmake_minimum_required(VERSION 2.8)

file(GLOB WE_SOURCES "*.cpp")
file(GLOB WE_HEADERS "*.h")

add_library(jsWrappers STATIC ${WE_SOURCES} ${WE_HEADERS} ../v8_wrapper.h ../v8_wrapper.cpp)
add_dependencies(jsWrappers html_dom)
target_link_libraries(jsWrappers html_dom)
