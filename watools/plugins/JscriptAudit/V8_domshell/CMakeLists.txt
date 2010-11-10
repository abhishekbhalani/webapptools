cmake_minimum_required(VERSION 2.8)

add_executable(v8_domshell
  stdafx.cpp
  stdafx.h
  V8_domshell.cpp
  ${jsWrappers_SOURCES}
  ${jsWrappers_HEADERS}
)

if(WIN32)
  target_link_libraries(v8_domshell webEngine jsWrappers
    Psapi.lib ws2_32.lib winmm.lib wldap32.lib
    optimized ${LOG4CXX_LIB_RELEASE} debug ${LOG4CXX_LIB_DEBUG} 
    optimized ${CURL_LIB_RELEASE}    debug ${CURL_LIB_DEBUG}
    optimized ${V8_LIB_RELEASE}      debug ${V8_LIB_DEBUG}
  )
  set_target_properties(v8_domshell PROPERTIES 
    LINK_FLAGS "/NODEFAULTLIB:LIBCMTD /NODEFAULTLIB:LIBCMT"
  )
endif()

