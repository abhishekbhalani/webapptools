
find_program(OMNIORB4_IDL_COMPILER
  NAMES omniidl
  PATHS ${OMNIORB4_DIR}/bin
        $ENV{OMNIORB4_DIR}/bin
        $ENV{OMNIORB4_DIR}/bin/x86_win32
        /usr/bin
        /usr/local/bin
  DOC "What is the path where omniidl (the idl compiler) can be found"
)

if(OMNIORB4_IDL_COMPILER)
  file(TO_NATIVE_PATH ${OMNIORB4_IDL_COMPILER} OMNIORB4_IDL_COMPILER)
  message(STATUS "Found OmniORB IDL compiler at ${OMNIORB4_IDL_COMPILER}")
endif()

find_program(ASTYLE_EXECUTABLE
  NAMES astyle
  DOC "path to astyle executable"
)

if(ASTYLE_EXECUTABLE)
  file(TO_NATIVE_PATH ${ASTYLE_EXECUTABLE} ASTYLE_EXECUTABLE)
  message(STATUS "Found AStyle at ${ASTYLE_EXECUTABLE}")
endif()

find_program(PYTHON_EXECUTABLE
  NAMES python
  DOC "path to python executable"
)

if(PYTHON_EXECUTABLE)
  file(TO_NATIVE_PATH ${PYTHON_EXECUTABLE} PYTHON_EXECUTABLE)
  message(STATUS "Found Python at ${PYTHON_EXECUTABLE}")
else()
  message(STATUS "Python executable not found!")
endif()

find_program(GCCXML_EXECUTABLE
  NAMES gccxml
  DOC "What is the path where gccxml can be found"
)

if(GCCXML_EXECUTABLE)
  file(TO_NATIVE_PATH ${GCCXML_EXECUTABLE} GCCXML_EXECUTABLE)
  message(STATUS "Found GCCXML at ${GCCXML_EXECUTABLE}")
endif()
