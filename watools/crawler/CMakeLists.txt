cmake_minimum_required(VERSION 2.8)

file(GLOB CRAWLER_SOURCES "*.cpp")
file(GLOB CRAWLER_HEADERS "*.h")
add_executable(crawler ${CRAWLER_SOURCES} ${CRAWLER_HEADERS} "../common/sysinfo.cpp")

if(WIN32)
  target_link_libraries(crawler webEngine 
    Psapi.lib ws2_32.lib winmm.lib wldap32.lib
    optimized ${LOG4CXX_LIB_RELEASE} debug ${LOG4CXX_LIB_DEBUG} 
    optimized ${CURL_LIB_RELEASE}    debug ${CURL_LIB_DEBUG}
  )
endif()

