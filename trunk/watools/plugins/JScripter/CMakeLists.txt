cmake_minimum_required(VERSION 2.8)

we_add_plugin(JScripter)

if(WIN32)
  target_link_libraries(JScripter jsWrappers
      Psapi.lib winmm.lib ws2_32.lib wldap32.lib
      optimized ${CURL_LIB_RELEASE}  debug ${CURL_LIB_DEBUG} 
      optimized ${V8_LIB_RELEASE}    debug ${V8_LIB_DEBUG}
  )
  set_target_properties(JScripter PROPERTIES 
    LINK_FLAGS "/NODEFAULTLIB:LIBCMTD /NODEFAULTLIB:LIBCMT"
  )
endif() 
