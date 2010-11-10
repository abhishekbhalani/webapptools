cmake_minimum_required(VERSION 2.8)

we_add_plugin(WoT)

if(WIN32)
  target_link_libraries(WoT
      Psapi.lib ws2_32.lib winmm.lib wldap32.lib
      optimized ${CURL_LIB_RELEASE}    debug ${CURL_LIB_DEBUG}
  )
endif()

