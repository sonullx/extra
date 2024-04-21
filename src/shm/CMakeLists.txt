add_library(extra_shm)
target_sources(extra_shm PRIVATE Shm.cpp)
target_link_libraries(extra_shm
	PRIVATE extra_basic
	PRIVATE extra_inner_header
)
