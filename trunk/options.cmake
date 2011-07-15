
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/../lib")

set(DOM_DIR_NAME "generated_dom")
set(DOM_DIRECTORY_TMP "${CMAKE_BINARY_DIR}/${DOM_DIR_NAME}")
set(DOM_DIRECTORY_SOURCE "${CMAKE_SOURCE_DIR}/../dom/${DOM_DIR_NAME}")
set(DOM_IMPLEMENTED_SOURCE "${DOM_DIRECTORY_SOURCE}/../implemented")
set(DOM_NOT_IMPLEMENTED_SOURCE "${DOM_DIRECTORY_SOURCE}/not_implemented")

include_directories(${DOM_DIRECTORY_SOURCE})

if(WIN32)
  if(MSVC)
    add_definitions(-D_SCL_SECURE_NO_WARNINGS -DV8_FAST_TLS -D_UNICODE -DUNICODE -D_WIN32)
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
  include_directories("${EXT_TOOLS}/include/soci")

### LOG4CXX ###
  find_library(LOG4CXX_LIB_DEBUG NAMES log4cxx PATHS "${EXT_LIBS_DEBUG}" "${EXT_LIBS_DEBUG}/log4cxx")
  find_library(LOG4CXX_LIB_RELEASE NAMES log4cxx PATHS "${EXT_LIBS_RELEASE}" "${EXT_LIBS_RELEASE}/log4cxx")
  file(GLOB log4cxx_additions_sources "${EXT_TOOLS}/include/log4cxx/*.cpp")
  list(LENGTH log4cxx_additions_sources log4cxx_additions_sources_len)
  if(${log4cxx_additions_sources_len} GREATER 0)
    add_definitions(-DLOG4CXX_WIDE_LOGGING)
    add_library(log4cxx_additions STATIC ${log4cxx_additions_sources})
    set(LOG4CXX_LIB_DEBUG ${LOG4CXX_LIB_DEBUG} log4cxx_additions)
    set(LOG4CXX_LIB_RELEASE ${LOG4CXX_LIB_RELEASE} log4cxx_additions)
  endif()

### CURL static ###
  find_library(CURL_LIB_DEBUG NAMES curl libcurl curllib PATHS "${EXT_LIBS_DEBUG}" "${EXT_LIBS_DEBUG}/curl")
  find_library(CURL_LIB_RELEASE NAMES curl libcurl curllib  PATHS "${EXT_LIBS_RELEASE}" "${EXT_LIBS_RELEASE}/curl")
  add_definitions(-DCURL_STATICLIB)

### V8 static ###
  find_library(V8_LIB_DEBUG NAMES v8 PATHS "${EXT_LIBS_DEBUG}/v8")
  find_library(V8_LIB_RELEASE NAMES v8 PATHS "${EXT_LIBS_RELEASE}/v8")

### SOCI ###
  find_library(SOCI_LIB NAMES soci_core soci_core_3_1 PATHS "${EXT_TOOLS}/lib/soci")

endif()

### BOOST ###
set(Boost_USE_STATIC_LIBS   ON)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost REQUIRED) 
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})


