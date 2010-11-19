###############################################################################
# CMake module to search for SQLite 3 library
#
# On success, the macro sets the following variables:
# SQLITE3_FOUND = if the library found
# SQLITE3_LIBRARY = full path to the library
# SQLITE3_LIBRARIES = full path to the library
# SSQLITE3_INCLUDE_DIR = where to find the library headers
#
# Copyright (c) 2009 Mateusz Loskot <mateusz@loskot.net>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
###############################################################################

if(WIN32)
  # TODO: Make use of OSGeo4W provider optional and available on explicit request
  set(OSGEO4W_IMPORT_LIBRARY sqlite3_i)
  if(DEFINED ENV{OSGEO4W_ROOT})
    set(OSGEO4W_ROOT_DIR $ENV{OSGEO4W_ROOT})
    message(STATUS
      "Trying OSGeo4W using environment variable OSGEO4W_ROOT=$ENV{OSGEO4W_ROOT}")
  else()
    set(OSGEO4W_ROOT_DIR c:/OSGeo4W)
    message(STATUS
      "Trying OSGeo4W using default location OSGEO4W_ROOT=${OSGEO4W_ROOT_DIR}")
  endif()
endif()

find_path(SQLITE3_INCLUDE_DIR
  NAMES sqlite3.h
  PATH_PREFIXES sqlite sqlite3
  PATHS
  /usr/include
  /usr/local/include
  $ENV{LIB_DIR}/include
  $ENV{LIB_DIR}/include/sqlite
  $ENV{LIB_DIR}/include/sqlite3
  $ENV{ProgramFiles}/SQLite/*/include
  $ENV{ProgramFiles}/SQLite3/*/include
  $ENV{SystemDrive}/SQLite/*/include
  $ENV{SystemDrive}/SQLite3/*/include
  ${OSGEO4W_ROOT_DIR}/include)

set(SQLITE3_NAMES ${OSGEO4W_IMPORT_LIBRARY} sqlite3)
find_library(SQLITE3_LIBRARY
  NAMES sqlite3
  PATHS
  /usr/lib
  /usr/local/lib
  $ENV{LIB_DIR}/lib
  $ENV{ProgramFiles}/SQLite/*/lib
  $ENV{ProgramFiles}/SQLite3/*/lib
  $ENV{SystemDrive}/SQLite/*/lib
  $ENV{SystemDrive}/SQLite3/*/lib
  ${OSGEO4W_ROOT_DIR}/lib)

set(SQLITE3_LIBRARIES ${SQLITE3_LIBRARY})

#message(STATUS ${SQLITE3_LIBRARY})
# Handle the QUIETLY and REQUIRED arguments and set SQLITE3_FOUND to TRUE
# if all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SQLITE3
  DEFAULT_MSG
  SQLITE3_LIBRARY
  SQLITE3_INCLUDE_DIR)

mark_as_advanced(SQLITE3_LIBRARY SQLITE3_INCLUDE_DIR SQLITE3_LIBRARY)