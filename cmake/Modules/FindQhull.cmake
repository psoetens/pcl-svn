###############################################################################
# Find QHULL 2011.1
#
# This sets the following variables:
# QHULL_FOUND - True if QHULL was found.
# QHULL_INCLUDE_DIRS - Directories containing the QHULL include files.
# QHULL_LIBRARIES - Libraries needed to use QHULL.
# QHULL_DEFINITIONS - Compiler flags for QHULL.

set(QHULL_MAJOR_VERSION 6)

find_path(QHULL_INCLUDE_DIR 
          NAMES libqhull/libqhull.h qhull.h
          HINTS "${QHULL_ROOT}" "$ENV{QHULL_ROOT}"
          PATHS "$ENV{PROGRAMFILES}/qhull 6.2.0.1373" "$ENV{PROGRAMW6432}/qhull 6.2.0.1373" 
          PATH_SUFFIXES qhull src/libqhull libqhull include)

# Prefer static libraries in Windows over shared ones
if(WIN32)
  find_library(QHULL_LIBRARY 
               NAMES qhullstatic qhull qhull${QHULL_MAJOR_VERSION}
               HINTS "${QHULL_ROOT}" "$ENV{QHULL_ROOT}"
               PATHS "$ENV{PROGRAMFILES}/qhull 6.2.0.1373" "$ENV{PROGRAMW6432}/qhull 6.2.0.1373" 
               PATH_SUFFIXES project build bin lib)

  find_library(QHULL_LIBRARY_DEBUG 
               NAMES qhullstatic_d qhull_d qhull_d${QHULL_MAJOR_VERSION} qhull qhull${QHULL_MAJOR_VERSION}
               HINTS "${QHULL_ROOT}" "$ENV{QHULL_ROOT}"
               PATHS "$ENV{PROGRAMFILES}/qhull 6.2.0.1373" "$ENV{PROGRAMW6432}/qhull 6.2.0.1373" 
               PATH_SUFFIXES project build bin lib)
else(WIN32)
  find_library(QHULL_LIBRARY 
               NAMES qhull qhull${QHULL_MAJOR_VERSION}
               HINTS "${QHULL_ROOT}" "$ENV{QHULL_ROOT}"
               PATH_SUFFIXES project build bin lib)

  find_library(QHULL_LIBRARY_DEBUG 
               NAMES qhull_d qhull_d${QHULL_MAJOR_VERSION} qhull qhull${QHULL_MAJOR_VERSION}
               HINTS "${QHULL_ROOT}" "$ENV{QHULL_ROOT}"
               PATH_SUFFIXES project build bin lib)
endif(WIN32)

if(NOT QHULL_LIBRARY_DEBUG)
  set(QHULL_LIBRARY_DEBUG ${QHULL_LIBRARY})
endif(NOT QHULL_LIBRARY_DEBUG)

set(QHULL_INCLUDE_DIRS ${QHULL_INCLUDE_DIR})
set(QHULL_LIBRARIES optimized ${QHULL_LIBRARY} debug ${QHULL_LIBRARY_DEBUG})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Qhull DEFAULT_MSG QHULL_LIBRARY
    QHULL_INCLUDE_DIR)

mark_as_advanced(QHULL_LIBRARY QHULL_LIBRARY_DEBUG QHULL_INCLUDE_DIR)

if(QHULL_FOUND)
  message(STATUS "QHULL found (include: ${QHULL_INCLUDE_DIRS}, lib: ${QHULL_LIBRARIES})")

	file(TO_CMAKE_PATH ${QHULL_LIBRARY} QHULL_LIBRARY_PATH)
	string(REGEX REPLACE "(.*)/[^/]+" "\\1" QHULL_LIBRARY_PATH "${QHULL_LIBRARY_PATH}")
	file(TO_CMAKE_PATH ${QHULL_LIBRARY_DEBUG} QHULL_LIBRARY_DEBUG_PATH)
	string(REGEX REPLACE "(.*)/[^/]+" "\\1" QHULL_LIBRARY_DEBUG_PATH "${QHULL_LIBRARY_DEBUG_PATH}")
	set(QHULL_LIB_DIRS ${QHULL_LIBRARY_PATH} ${QHULL_LIBRARY_DEBUG_PATH} 
		CACHE INTERNAL "QHULL_LIB_DIRS" FORCE)
	set(QHULL_INCLUDE_DIRS ${QHULL_INCLUDE_DIRS}
		CACHE INTERNAL "QHULL_INCLUDE_DIRS" FORCE)
endif(QHULL_FOUND)
