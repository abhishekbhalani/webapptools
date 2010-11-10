cmake_minimum_required(VERSION 2.8)

file(GLOB connector_SOURCES "*.cpp")
file(GLOB connector_HEADERS "*.h")
add_executable(connector ${connector_SOURCES} ${connector_HEADERS} "../common/sysinfo.cpp")

if(WIN32)
  target_link_libraries(connector webEngine 
    Psapi.lib ws2_32.lib winmm.lib wldap32.lib
    optimized ${LOG4CXX_LIB_RELEASE} debug ${LOG4CXX_LIB_DEBUG} 
    optimized ${CURL_LIB_RELEASE}    debug ${CURL_LIB_DEBUG}
  )
endif()

