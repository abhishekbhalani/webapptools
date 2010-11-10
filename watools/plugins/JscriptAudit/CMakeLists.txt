cmake_minimum_required(VERSION 2.8)

we_add_plugin(aJscript)

if(WIN32)
  target_link_libraries(aJscript jsWrappers
      Psapi.lib ws2_32.lib winmm.lib wldap32.lib
      optimized ${CURL_LIB_RELEASE}  debug ${CURL_LIB_DEBUG}
      optimized ${V8_LIB_RELEASE}    debug ${V8_LIB_DEBUG}
  )
  set_target_properties(aJscript PROPERTIES 
    LINK_FLAGS "/NODEFAULTLIB:LIBCMTD /NODEFAULTLIB:LIBCMT"
  )
endif()

if(BUILD_DOMSHELL)
  add_subdirectory(V8_domshell)
endif()
