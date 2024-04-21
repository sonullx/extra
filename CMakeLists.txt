cmake_minimum_required(VERSION 3.14)
project(extra
	VERSION 0.1
	DESCRIPTION "A Trading System"
	HOMEPAGE_URL https://github.com/sonullx/extra
	LANGUAGES CXX C
)

add_library(extra_basic INTERFACE)
target_compile_features(extra_basic INTERFACE cxx_std_20)
target_compile_features(extra_basic INTERFACE c_std_11)

add_library(extra_inner_header INTERFACE)
target_include_directories(extra_inner_header INTERFACE include/extra)

add_library(extra_outer_header INTERFACE)
target_include_directories(extra_outer_header INTERFACE include)

add_subdirectory(src)
add_subdirectory(3rd)
add_subdirectory(test)
