add_library(extra_protocol)
target_sources(extra_protocol PRIVATE HttpV1D0.cpp HttpV1D1.cpp HttpV2.cpp)
target_link_libraries(extra_protocol PRIVATE extra_basic)
