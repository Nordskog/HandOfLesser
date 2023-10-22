add_library(HandOfLesserCommon STATIC
	src/transport/nativetransport.cpp
	src/transport/transport.cpp
	src/transport/transportutil.cpp
	src/transport/udptransport.cpp
	src/math/fingers.cpp
	src/math/utils.cpp
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

add_executable(HandOfLesserCommon.Tests
	tests/test_example.cpp
)

target_link_libraries(HandOfLesserCommon.Tests PRIVATE
	gtest
	gtest_main
)

gtest_discover_tests(HandOfLesserCommon.Tests)
