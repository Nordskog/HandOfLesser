cmake_minimum_required (VERSION 3.8)

project(HandOfLesserDriver)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../output/drivers")

add_definitions(-D_WIN32)

set(ARCH_TARGET "win64") # This is where Steam looks for drivers on windows

add_subdirectory(utils)
add_subdirectory(HandOfLesserDriver)
