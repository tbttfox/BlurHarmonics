# - Find DirectInput
# Find the DirectSound includes and libraries
#
#  MAXSDK_INCLUDE_DIR - where to find baseinterface.h
#  MAXSDK_LIB_DIR     - Library directory of 3DSMAX
#  MAXSDK_LIBRARIES   - List of libraries when using 3DSMAX.
#  MAXSDK_FOUND       - True if MAX SDK found.

if(MAXSDK_INCLUDE_DIR)
    # Already in cache, be silent
    set(MAXSDK_FIND_QUIETLY TRUE)
endif(MAXSDK_INCLUDE_DIR)

# Set a default Maya version if not specified
if(NOT DEFINED MAX_VERSION)
  set(MAX_VERSION 2016 CACHE STRING "Max version")
endif()

find_path(MAXSDK_INCLUDE_DIR max.h 
  PATHS
  "$ENV{ADSK_3DSMAX_SDK_${MAX_VERSION}}/include"
  "$ENV{ADSK_3DSMAX_SDK_${MAX_VERSION}}/maxsdk/include"
  "$ENV{PROGRAMFILES}/Autodesk/3ds Max ${MAX_VERSION} SDK/maxsdk/include"
) 

find_path(MAXSDK_CS_INCLUDE_DIR bipexp.h
  PATHS
  "$ENV{ADSK_3DSMAX_SDK_${MAX_VERSION}}/include/CS"
  "$ENV{ADSK_3DSMAX_SDK_${MAX_VERSION}}/maxsdk/include/CS"
  "$ENV{PROGRAMFILES}/Autodesk/3ds Max ${MAX_VERSION} SDK/maxsdk/include/CS"
)

find_path(MAXSDK_LIB_DIR core.lib
  PATHS
  "$ENV{ADSK_3DSMAX_SDK_${MAX_VERSION}}/lib/x64/Release"
  "$ENV{ADSK_3DSMAX_SDK_${MAX_VERSION}}/maxsdk/lib/x64/Release"
  "$ENV{PROGRAMFILES}/Autodesk/3ds Max ${MAX_VERSION} SDK/maxsdk/lib/x64/Release"
)

FIND_LIBRARY(MAXSDK_CORE_LIBRARY NAMES core PATHS ${MAXSDK_LIB_DIR})
FIND_LIBRARY(MAXSDK_GEOM_LIBRARY NAMES geom PATHS ${MAXSDK_LIB_DIR})
FIND_LIBRARY(MAXSDK_GFX_LIBRARY NAMES gfx PATHS ${MAXSDK_LIB_DIR})
FIND_LIBRARY(MAXSDK_MESH_LIBRARY NAMES mesh PATHS ${MAXSDK_LIB_DIR})
FIND_LIBRARY(MAXSDK_MAXUTIL_LIBRARY NAMES maxutil PATHS ${MAXSDK_LIB_DIR})
FIND_LIBRARY(MAXSDK_MAXSCRIPT_LIBRARY NAMES Maxscrpt PATHS ${MAXSDK_LIB_DIR})
FIND_LIBRARY(MAXSDK_PARAMBLK2_LIBRARY NAMES Paramblk2 PATHS ${MAXSDK_LIB_DIR})
FIND_LIBRARY(MAXSDK_BMM_LIBRARY NAMES bmm PATHS ${MAXSDK_LIB_DIR})

# Handle the QUIETLY and REQUIRED arguments and set MAXSDK_FOUND to TRUE if
# all listed variables are TRUE.
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MAXSDK DEFAULT_MSG
    MAXSDK_INCLUDE_DIR MAXSDK_CORE_LIBRARY)

if(MAXSDK_FOUND)
    SET(MAXSDK_LIBRARIES
      ${MAXSDK_CORE_LIBRARY}
      ${MAXSDK_GEOM_LIBRARY}
      ${MAXSDK_GFX_LIBRARY}
      ${MAXSDK_MESH_LIBRARY}
      ${MAXSDK_MAXUTIL_LIBRARY}
      ${MAXSDK_MAXSCRIPT_LIBRARY}
      ${MAXSDK_PARAMBLK2_LIBRARY}
      ${MAXSDK_BMM_LIBRARY} )

else(MAXSDK_FOUND)
    set(MAXSDK_LIBRARIES)
endif(MAXSDK_FOUND)

mark_as_advanced(MAXSDK_INCLUDE_DIR MAXSDK_LIB_DIR MAXSDK_LIBRARIES)
