add_executable(HandOfLesser
	src/display_global.cpp
	src/HandOfLesser.cpp
	src/HandOfLesserCore.cpp
	src/HandTracking.cpp
	src/HandTrackingInterface.cpp
	src/InstanceHolder.cpp
	src/math_utils.cpp
	src/settings_global.cpp
	src/simple_gesture_detector.cpp
	src/openxr_hand.cpp
	src/user_interface.cpp
	src/XrUtils.cpp
	src/openxr_hacks.cpp
)

find_package(OpenGL REQUIRED)

target_link_libraries(HandOfLesser PRIVATE
	d3d11
	eigen
	glfw
	HandOfLesserCommon
	imgui
	openxr_loader
	openxr-hpp
	${OPENGL_LIBRARY}
	${OPENVR_LIBRARIES}
)

target_include_directories(HandOfLesser PRIVATE ${OPENVR_INCLUDE_DIR})

target_compile_definitions(HandOfLesser PUBLIC
	NOMINMAX
	WIN32_LEAN_AND_MEAN
	XR_KHR_D3D11_enable
	XR_USE_GRAPHICS_API_D3D11
	XR_USE_PLATFORM_WIN32
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../output)
set(ARCH_TARGET "win64")
set_target_properties(HandOfLesser PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY $<1:${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/bin/${ARCH_TARGET}>
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
	set_property(TARGET HandOfLesser PROPERTY CXX_STANDARD 20)
endif()
