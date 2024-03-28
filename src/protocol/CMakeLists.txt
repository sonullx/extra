add_library(extra_protocol)
target_sources(extra_protocol PRIVATE HttpV1.cpp)
target_link_libraries(extra_protocol PUBLIC extra_basic)
