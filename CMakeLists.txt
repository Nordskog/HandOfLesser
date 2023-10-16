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

## Needed for OpenXR math
add_library(openxr-common INTERFACE)
target_include_directories(openxr-common INTERFACE lib/openxr_sdk/src/common)

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

# -------------- IMGUI static library creation`
#
set(IMGUI_PATH ${CMAKE_CURRENT_LIST_DIR}/lib/imgui)
file(GLOB IMGUI_GLOB
    ${IMGUI_PATH}/imgui.h
    ${IMGUI_PATH}/imgui.cpp
    ${IMGUI_PATH}/imconfig.h
    ${IMGUI_PATH}/imgui_demo.cpp
    ${IMGUI_PATH}/imgui_draw.cpp
    ${IMGUI_PATH}/imgui_internal.h
    ${IMGUI_PATH}/imstb_rectpack.h
    ${IMGUI_PATH}/imstb_textedit.h
    ${IMGUI_PATH}/imstb_truetype.h
    ${IMGUI_PATH}/imgui_tables.cpp
    ${IMGUI_PATH}/imgui_widgets.cpp

    # specific bindings...
    ${IMGUI_PATH}/backends/imgui_impl_win32.cpp
    ${IMGUI_PATH}/backends/imgui_impl_glfw.h
    ${IMGUI_PATH}/backends/imgui_impl_glfw.cpp
    ${IMGUI_PATH}/backends/imgui_impl_opengl3.h
    ${IMGUI_PATH}/backends/imgui_impl_opengl3.cpp
    ${IMGUI_PATH}/backends/imgui_impl_opengl3_loader.cpp)
add_library("imgui" STATIC ${IMGUI_GLOB})
target_include_directories("imgui" PUBLIC ${IMGUI_PATH})
target_include_directories("imgui" PUBLIC ${IMGUI_PATH}/backends)
target_link_libraries("imgui" PRIVATE glfw)
# ----------------------------------------------------------------------

# Include sub-projects.
add_subdirectory("lib/openxr_sdk")
add_subdirectory("lib/openvr")
add_subdirectory("lib/glfw")
add_subdirectory("HandOfLesserCommon")
add_subdirectory("HandOfLesser")
add_subdirectory("HandOfLesserDriver")

