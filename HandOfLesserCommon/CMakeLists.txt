add_library(HandOfLesserCommon STATIC
	src/transport/nativetransport.cpp
	src/transport/transport.cpp
	src/transport/transportutil.cpp
	src/transport/udptransport.cpp
	src/math/fingers.cpp
	src/math/math_utils.cpp
	src/hand/finger_bend.cpp
	src/controller/controller.cpp
)

target_link_libraries(HandOfLesserCommon PUBLIC
	eigen
	openxr_loader
	wsock32
)

target_include_directories(HandOfLesserCommon
	INTERFACE include
	PRIVATE ${OPENVR_INCLUDE_DIR}
)

# Allow us to include from src dir path instead of using relative paths
target_include_directories(HandOfLesserCommon PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(HandOfLesserCommon.Tests
	tests/test_example.cpp
)

target_link_libraries(HandOfLesserCommon.Tests PRIVATE
	gtest
	gtest_main
)

gtest_discover_tests(HandOfLesserCommon.Tests)
