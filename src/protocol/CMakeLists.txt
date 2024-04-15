add_library(extra_protocol)
target_sources(extra_protocol PRIVATE HttpBasic.cpp HttpSession.cpp)
target_link_libraries(extra_protocol PRIVATE extra_basic)
