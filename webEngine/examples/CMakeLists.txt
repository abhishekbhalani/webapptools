cmake_minimum_required(VERSION 2.8)

macro(we_add_example EXAMPLE_NAME)
  set(OPTIONAL_SOURCES ${ARGV})
  list(REMOVE_AT OPTIONAL_SOURCES 0)

  add_executable(${EXAMPLE_NAME} "${EXAMPLE_NAME}/${EXAMPLE_NAME}.cpp" ${OPTIONAL_SOURCES})

  if(WIN32)
    target_link_libraries(${EXAMPLE_NAME} webEngine 
      optimized ${LOG4CXX_LIB_RELEASE} debug ${LOG4CXX_LIB_DEBUG} 
    )
  endif()
  set_target_properties(${EXAMPLE_NAME} PROPERTIES 
    FOLDER examples
  )
endmacro(we_add_example)

we_add_example(difflib)

we_add_example(document)

we_add_example(httpget)
target_link_libraries(httpget
    Psapi.lib ws2_32.lib winmm.lib wldap32.lib
    optimized ${CURL_LIB_RELEASE}    debug ${CURL_LIB_DEBUG}
)

we_add_example(tagscanner)

we_add_example(task)
target_link_libraries(task
    Psapi.lib ws2_32.lib winmm.lib wldap32.lib
    optimized ${CURL_LIB_RELEASE}    debug ${CURL_LIB_DEBUG}
)






