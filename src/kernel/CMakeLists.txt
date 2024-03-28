add_library(extra_kernel)
target_sources(extra_kernel PRIVATE poller.c)
target_link_libraries(extra_kernel PUBLIC extra_basic)