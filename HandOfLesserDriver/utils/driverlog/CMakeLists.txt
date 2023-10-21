add_library(util_driverlog STATIC
	driverlog.cpp
)

target_include_directories(util_driverlog PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
	${OPENVR_INCLUDE_DIR}
)

target_link_libraries(util_driverlog PRIVATE
	${OPENVR_LIBRARIES}
)
