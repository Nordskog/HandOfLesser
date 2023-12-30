add_executable(HandOfLesser
	src/core/ui/display_global.cpp
	src/HandOfLesser.cpp
	src/core/HandOfLesserCore.cpp
	src/openxr/HandTracking.cpp
	src/openxr/HandTrackingInterface.cpp
	src/openxr/InstanceHolder.cpp

	src/core/settings_global.cpp
	src/hands/simple_gesture_detector.cpp
	src/openxr/openxr_hand.cpp
	src/core/ui/user_interface.cpp
	src/openxr/XrUtils.cpp
	src/oculus/oculus_hacks.cpp
	src/vrchat/vrchat_osc.cpp
	src/openxr/openxr_state.cpp
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

# Allow us to include from src dir path instead of using relative paths
target_include_directories(HandOfLesser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

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
