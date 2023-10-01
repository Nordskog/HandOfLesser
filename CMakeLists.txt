# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("HandOfLesser")
set(CMAKE_CXX_STANDARD 14)

## Needed for OpenXR
add_library(openxr-hpp INTERFACE)
target_include_directories(openxr-hpp INTERFACE lib/openxr-hpp)

## Needed for OpenVR
set(OPENVR_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/openvr")
set(OPENVR_INCLUDE_DIR "${OPENVR_LIB_DIR}/headers")

message( "\${OPENVR_LIB_DIR}: ${OPENVR_LIB_DIR}" )
message( "\${OPENVR_INCLUDE_DIR}: ${OPENVR_INCLUDE_DIR}" )

# Doesn't update unless explicitly cleared first. gg'
unset(OPENVR_LIBRARIES CACHE) 

find_library(OPENVR_LIBRARIES
  NAMES
    openvr_api
  PATHS
    ${OPENVR_LIB_DIR}/bin
    ${OPENVR_LIB_DIR}/lib
  PATH_SUFFIXES
    win64
  NO_DEFAULT_PATH
  NO_CMAKE_FIND_ROOT_PATH
)

message( "\${OPENVR_LIBRARIES}: ${OPENVR_LIBRARIES}" )

# Include sub-projects.
add_subdirectory("lib/openxr_sdk")
add_subdirectory("lib/openvr")
add_subdirectory("HandOfLesserCommon")
add_subdirectory("HandOfLesser")
add_subdirectory("HandOfLesserDriver")

