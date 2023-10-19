add_executable(HandOfLesser
    src/HandOfLesser.cpp
    src/InstanceHolder.cpp
    src/XrUtils.cpp
    src/HandTracking.cpp
    src/HandTrackingInterface.cpp
    src/TrackedHand.cpp
    src/HandOfLesserCore.cpp
    src/simple_gesture_detector.cpp
    src/user_interface.cpp
    src/settings_global.cpp
)

find_package(OpenGL REQUIRED)

target_link_libraries(HandOfLesser PRIVATE
    HandOfLesserCommon
    ${OPENVR_LIBRARIES}
    openxr_loader
    d3d11
    openxr-hpp
    openxr-common
    glfw
    imgui
    ${OPENGL_LIBRARY}
)

target_link_libraries(HandOfLesser PRIVATE ${OPENVR_LIBRARIES})
target_include_directories(HandOfLesser PRIVATE ${OPENVR_INCLUDE_DIR})

# openxr math functions live here
#target_include_directories(HandOfLesser PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../lib/openxr_sdk/src/common)

target_compile_definitions(HandOfLesser PUBLIC XR_USE_PLATFORM_WIN32)
target_compile_definitions(HandOfLesser PUBLIC XR_USE_GRAPHICS_API_D3D11)
target_compile_definitions(HandOfLesser PUBLIC XR_KHR_D3D11_enable)
target_compile_definitions(HandOfLesser PUBLIC NOMINMAX)
target_compile_definitions(HandOfLesser PUBLIC WIN32_LEAN_AND_MEAN)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../output/")
set(ARCH_TARGET "win64") 
set_target_properties(HandOfLesser PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY $<1:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/bin/${ARCH_TARGET}>
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
    set_property(TARGET HandOfLesser PROPERTY CXX_STANDARD 20)
endif()
