add_library(extra_channel INTERFACE)
# target_precompile_headers(extra_channel INTERFACE Channel.h)
target_link_libraries(extra_channel
	INTERFACE extra_basic
	INTERFACE extra_inner_header
	INTERFACE rt
)