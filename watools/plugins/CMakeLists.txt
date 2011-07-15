cmake_minimum_required(VERSION 2.8)

macro(we_add_plugin PLUGIN_NAME)
  file(GLOB PLUGIN_SOURCES "*.cpp")
  file(GLOB PLUGIN_HEADERS "*.h")

  set(OPTIONAL_SOURCES ${ARGV})
  list(REMOVE_AT OPTIONAL_SOURCES 0)

  add_library(${PLUGIN_NAME} SHARED ${PLUGIN_SOURCES} ${PLUGIN_HEADERS} ${OPTIONAL_SOURCES} dlExport.def)
  set_target_properties(${PLUGIN_NAME} PROPERTIES PREFIX "" SUFFIX ".plg")

  if(WIN32)
    target_link_libraries(${PLUGIN_NAME} webEngine 
      optimized ${LOG4CXX_LIB_RELEASE} debug ${LOG4CXX_LIB_DEBUG} 
    )
    set_target_properties(${PLUGIN_NAME} PROPERTIES 
      FOLDER plugins
    )
  endif()
endmacro(we_add_plugin)

add_subdirectory(../common/jsWrappers jsWrappers)

add_subdirectory(WoT)
add_subdirectory(JscriptAudit)
add_subdirectory(Soci)

if(BUILD_EXAMPLES)
  add_subdirectory(Demo)
endif()



