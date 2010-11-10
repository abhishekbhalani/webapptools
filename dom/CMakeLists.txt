cmake_minimum_required(VERSION 2.8)

include("programs.cmake")

#set(FROM_IDL "${CMAKE_CURRENT_SOURCE_DIR}/level_1/html.idl")
set(FROM_IDL "${CMAKE_CURRENT_SOURCE_DIR}/level_2/level_2.idl")

file(MAKE_DIRECTORY ${DOM_DIRECTORY_TMP})
file(MAKE_DIRECTORY ${DOM_DIRECTORY_SOURCE})
file(MAKE_DIRECTORY ${DOM_IMPLEMENTED_SOURCE})
file(MAKE_DIRECTORY ${DOM_NOT_IMPLEMENTED_SOURCE})

include_directories(${DOM_DIRECTORY_SOURCE})

set(IDL_HEADER "${DOM_DIRECTORY_SOURCE}/html.h")
set(IDL_SOURCE "${DOM_DIRECTORY_SOURCE}/html.cpp")
set(JS_HEADER  "${DOM_DIRECTORY_SOURCE}/html_js.h")
set(JS_SOURCE  "${DOM_DIRECTORY_SOURCE}/html_js.cpp")
set(TAGS_WRAPPER  "${DOM_DIRECTORY_SOURCE}/html_tags_wrapper.h")

set(GENERATED_DOM_SOURCES "${IDL_HEADER}" "${IDL_SOURCE}" "${JS_HEADER}" "${JS_SOURCE}" "${TAGS_WRAPPER}")
file(GLOB HTML_DOM_IMPL "${DOM_IMPLEMENTED_SOURCE}/*.cpp")
file(GLOB HTML_DOM_NOT_IMPL "${DOM_NOT_IMPLEMENTED_SOURCE}/*.cpp")

source_group(dom FILES ${GENERATED_DOM_SOURCES})
source_group(implemented FILES ${HTML_DOM_IMPL})
source_group(not_implemented FILES ${HTML_DOM_NOT_IMPL})

add_library(html_dom STATIC ${GENERATED_DOM_SOURCES} ${HTML_DOM_IMPL} ${HTML_DOM_NOT_IMPL})

if(NEED_GENERATE_DOM)
  if(NOT DEFINED ASTYLE_EXECUTABLE)
    set(ASTYLE_EXECUTABLE "echo")
  endif()

  add_custom_target(generating_sources_from_idl)

  add_custom_command(  
    TARGET generating_sources_from_idl
    PRE_BUILD
    COMMAND "${OMNIORB4_IDL_COMPILER}" -bdump -nc ${FROM_IDL} | "${PYTHON_EXECUTABLE}" ${CMAKE_CURRENT_SOURCE_DIR}/wrap_idl.py ${DOM_DIRECTORY_SOURCE}
    COMMAND "${GCCXML_EXECUTABLE}" -I "${EXT_TOOLS}/include" --gccxml-compiler msvc9 ${IDL_HEADER} "-fxml=${DOM_DIRECTORY_TMP}/html.xml"
    COMMAND "${PYTHON_EXECUTABLE}" ${CMAKE_CURRENT_SOURCE_DIR}/generate_sources.py ${DOM_DIRECTORY_SOURCE} ${DOM_DIRECTORY_TMP} ${DOM_IMPLEMENTED_SOURCE} ${DOM_NOT_IMPLEMENTED_SOURCE}
    COMMAND "${ASTYLE_EXECUTABLE}" --style=stroustrup --indent=spaces=4 --suffix=none --quiet "${DOM_DIRECTORY_SOURCE}/*.cpp" "${DOM_DIRECTORY_SOURCE}/*.h" "${DOM_NOT_IMPLEMENTED_SOURCE}/*.cpp"
    DEPENDS ${FROM_IDL}
  )
endif()


