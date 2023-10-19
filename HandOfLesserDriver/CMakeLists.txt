cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project (HandOfLesserDriver)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../output/drivers")

add_definitions(-D_WIN32)

set(ARCH_TARGET "win64") # This is where steam looks for it on windows

#cd to ... Steam\steamapps\common\SteamVR\bin\win64
#./vrpathreg.exe adddriver " ... output\drivers\simplecontroller"
# ultimately move to steamVR drivers but this is easier while dev'ing

add_subdirectory(utils)
add_subdirectory(HandOfLesserDriver)
