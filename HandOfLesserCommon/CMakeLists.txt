add_library(HandOfLesserCommon STATIC
	src/transport/nativetransport.cpp
	src/transport/transport.cpp
	src/transport/transportutil.cpp
	src/transport/udptransport.cpp
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
