add_library(HandOfLesserCommon STATIC
	src/transport/udptransport.cpp
	src/transport/transport.cpp
	src/transport/transportutil.cpp
	src/transport/nativetransport.cpp
)

target_link_libraries(HandOfLesserCommon PUBLIC
	openxr_loader
	wsock32
	eigen
)

#set(HAND_OF_LESSER_COMMON_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")

message( "HandOfLesserCommon include dir" "${CMAKE_CURRENT_SOURCE_DIR}/include" )

target_include_directories(HandOfLesserCommon INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(HandOfLesserCommon PRIVATE ${OPENVR_INCLUDE_DIR})
