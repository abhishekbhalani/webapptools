
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set(DOM_DIR_NAME "generated_dom")
set(DOM_DIRECTORY_TMP "${CMAKE_BINARY_DIR}/${DOM_DIR_NAME}")
set(DOM_DIRECTORY_SOURCE "${CMAKE_SOURCE_DIR}/../dom/${DOM_DIR_NAME}")
set(DOM_IMPLEMENTED_SOURCE "${DOM_DIRECTORY_SOURCE}/../implemented")
set(DOM_NOT_IMPLEMENTED_SOURCE "${DOM_DIRECTORY_SOURCE}/not_implemented")

include_directories(${DOM_DIRECTORY_SOURCE})

if(WIN32)
  if(MSVC)
    add_definitions(-D_SCL_SECURE_NO_WARNINGS)
  endif()

  set(EXT_TOOLS "${CMAKE_SOURCE_DIR}/../ext_tools")
  set(BOOST_ROOT "${EXT_TOOLS}")
  set(EXT_LIBS_DEBUG "${EXT_TOOLS}/lib/Debug")
  set(EXT_LIBS_RELEASE "${EXT_TOOLS}/lib/Release")

### LIB ###
  link_directories("${EXT_LIBS_DEBUG}/boost" "${EXT_LIBS_RELEASE}/boost")

### INCLUDE ###
  include_directories("${EXT_TOOLS}/include")
  include_directories("${EXT_TOOLS}/include/v8")
  include_directories("../soci/src/core")

### LOG4CXX ###
  find_library(LOG4CXX_LIB_DEBUG NAMES log4cxx PATHS "${EXT_LIBS_DEBUG}" "${EXT_LIBS_DEBUG}/log4cxx")
  find_library(LOG4CXX_LIB_RELEASE NAMES log4cxx PATHS "${EXT_LIBS_RELEASE}" "${EXT_LIBS_RELEASE}/log4cxx")

### CURL static ###
  find_library(CURL_LIB_DEBUG NAMES curl libcurl curllib PATHS "${EXT_LIBS_DEBUG}" "${EXT_LIBS_DEBUG}/curl")
  find_library(CURL_LIB_RELEASE NAMES curl libcurl curllib  PATHS "${EXT_LIBS_RELEASE}" "${EXT_LIBS_RELEASE}/curl")
  add_definitions(-DCURL_STATICLIB)

### V8 static ###
  find_library(V8_LIB_DEBUG NAMES v8 PATHS "${EXT_LIBS_DEBUG}")
  find_library(V8_LIB_RELEASE NAMES v8 PATHS "${EXT_LIBS_RELEASE}")

### SOCI dynamic ###
  add_subdirectory("../soci/src" soci)
  set_target_properties(soci_core soci_sqlite3 PROPERTIES 
    FOLDER soci
    COMPILE_DEFINITIONS SOCI_DLL,SOCI_USE_LOG4CXX,SOCI_USE_BOOST
  )
  set_target_properties(soci_core PROPERTIES
    COMPILE_DEFINITIONS CORE_EXPORTS,SOCI_DLL,SOCI_USE_LOG4CXX,SOCI_USE_BOOST
  )
endif()

### BOOST ###
set(Boost_USE_STATIC_LIBS   ON)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost REQUIRED) 
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})


