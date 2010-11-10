################################################################################
#
# This file is part of CMake configuration for SOCI library
#
# Copyright (C) 2009-2010 Mateusz Loskot <mateusz@loskot.net>
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt)
#
################################################################################

colormsg(_HIBLUE_ "Configuring SOCI core library:")
message(STATUS "")

# Core dependencies
set(SOCI_CORE_DEPENDENCIES)

# TODO: disable on windows --mloskot
set(DL_FIND_QUIETLY TRUE)
find_package(DL)
if(DL_FOUND)
  list(APPEND SOCI_CORE_DEPENDENCIES ${DL_LIBRARY})
  set_directory_properties(PROPERTIES
    INCLUDE_DIRECTORIES ${DL_INCLUDE_DIR})
  add_definitions(-DHAVE_DL=1)
endif()

# Core source files
file(GLOB SOCI_CORE_HEADERS *.h)
file(GLOB SOCI_CORE_SOURCES *.cpp)

# Core targets
string(TOLOWER "${PROJECT_NAME}" PROJECTNAMEL)
set(SOCI_CORE_TARGET ${PROJECTNAMEL}_core)

add_library(${SOCI_CORE_TARGET} SHARED ${SOCI_CORE_SOURCES})

target_link_libraries(${SOCI_CORE_TARGET} ${SOCI_CORE_DEPENDENCIES})
if(WIN32)
  target_link_libraries(${SOCI_CORE_TARGET}
    optimized ${LOG4CXX_LIB_RELEASE} debug ${LOG4CXX_LIB_DEBUG} 
  )
endif()

set_target_properties(${SOCI_CORE_TARGET}
  PROPERTIES
  CLEAN_DIRECT_OUTPUT 1
  VERSION ${SOCI_VERSION}
  SOVERSION ${SOCI_SOVERSION})

# install
INSTALL(FILES ${SOCI_CORE_HEADERS} DESTINATION ${INCLUDEDIR}/${PROJECTNAMEL})


# Summary
boost_report_value(SOCI_CORE_TARGET)
boost_report_value(SOCI_CORE_DEPENDENCIES)
boost_report_value(WITH_BOOST)
soci_report_directory_property(COMPILE_DEFINITIONS)

# Export core target name to make it visibla by backends
set(SOCI_CORE_TARGET ${SOCI_CORE_TARGET} PARENT_SCOPE)
set(SOCI_CORE_STATIC_DEPENDENCIES ${SOCI_CORE_DEPENDENCIES} PARENT_SCOPE)
