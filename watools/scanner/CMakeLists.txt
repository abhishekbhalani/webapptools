cmake_minimum_required(VERSION 2.8)

file(GLOB scanner_SOURCES "*.cpp")
file(GLOB scanner_HEADERS "*.h")
add_executable(scanner ${scanner_SOURCES} ${scanner_HEADERS} "../common/sysinfo.cpp")

if(WIN32)
  target_link_libraries(scanner webEngine 
    Psapi.lib ws2_32.lib winmm.lib wldap32.lib
    optimized ${LOG4CXX_LIB_RELEASE} debug ${LOG4CXX_LIB_DEBUG} 
    optimized ${CURL_LIB_RELEASE}    debug ${CURL_LIB_DEBUG}
  )
endif()

