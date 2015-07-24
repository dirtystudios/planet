# Locate the LIBNOISE library
# This module defines the following variables:
# LIBNOISE_LIBRARY, the name of the library;
# LIBNOISE_INCLUDE_DIR, where to find LIBNOISE include files.
# LIBNOISE_FOUND, true if both the LIBNOISE_LIBRARY and LIBNOISE_INCLUDE_DIR have been found.
#
# To help locate the library and include file, you could define an environment variable called
# LIBNOISE_ROOT which points to the root of the LIBNOISE library installation. This is pretty useful
# on a Windows platform.
#
#
# Usage example to compile an "executable" target to the LIBNOISE library:
#
# FIND_PACKAGE (libnoise REQUIRED)
# INCLUDE_DIRECTORIES (${LIBNOISE_INCLUDE_DIR})
# ADD_EXECUTABLE (executable ${EXECUTABLE_SRCS})
# TARGET_LINK_LIBRARIES (executable ${LIBNOISE_LIBRARY})
#
# TODO:
# Allow the user to select to link to a shared library or to a static library.

#Search for the include file...
FIND_PATH(LIBNOISE_INCLUDE_DIRS noise/noise.h DOC "Path to libnoise include directory."
  HINTS
  $ENV{LIBNOISE_ROOT}
  PATH_SUFFIX include #For finding the include file under the root of the glfw expanded archive, typically on Windows.
  PATHS
  /usr/include/
  /usr/local/include/
  # By default headers are under GLFW subfolder
  /usr/include/noise
  /usr/local/include/noise
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${LIBNOISE_ROOT}/include/ # added by ptr
)

SET(LIBNOISE_LIB_NAMES libnoise.a libnoise.lib)

FIND_LIBRARY(LIBNOISE_LIBRARIES DOC "Absolute path to LIBNOISE library."
  NAMES ${LIBNOISE_LIB_NAMES}
  HINTS
  $ENV{LIBNOISE_ROOT}
  PATH_SUFFIXES lib/win32 #For finding the library file under the root of the LIBNOISE expanded archive, typically on Windows.
  PATHS
  /usr/local/lib
  /usr/lib
  ${CMAKE_CURRENT_SOURCE_DIR}/winlibs
  ${LIBNOISE_ROOT_DIR}/lib-msvc100/release # added by ptr
)

IF(LIBNOISE_LIBRARIES AND LIBNOISE_INCLUDE_DIRS)
  SET(LIBNOISE_FOUND TRUE)
  message(STATUS "Found LIBNOISE: ${LIBNOISE_LIBRARIES}")
ELSE()
  message(STATUS "LIBNOISE3 NOT found!")
ENDIF(LIBNOISE_LIBRARIES AND LIBNOISE_INCLUDE_DIRS)

#if(LIBNOISE_FOUND)
#  MARK_AS_ADVANCED(LIBNOISE_INCLUDE_DIRS LIBNOISE_LIBRARIES)
#endif(LIBNOISE_FOUND)