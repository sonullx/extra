enable_testing()
add_executable(extra_protocol_test)
target_sources(extra_protocol_test PRIVATE protocol_test.cpp)
target_link_libraries(extra_protocol_test
	PRIVATE extra_basic
	PRIVATE extra_outer_header
	PRIVATE extra_protocol
	PRIVATE GTest::gtest_main
)
add_executable(extra_channel_test)
target_sources(extra_channel_test PRIVATE channel_test.cpp)
target_link_libraries(extra_channel_test
	PRIVATE extra_basic
	PRIVATE extra_outer_header
	PRIVATE extra_channel
	PRIVATE GTest::gtest_main
)
include(GoogleTest)
gtest_discover_tests(extra_protocol_test)
gtest_discover_tests(extra_channel_test)
