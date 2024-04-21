cmake_minimum_required(VERSION 3.14)
PROJECT(extra
	VERSION 0.1
	DESCRIPTION "EXchange TRAding system"
	HOMEPAGE_URL https://github.com/sonullx/extra
	LANGUAGES CXX C
)

add_library(extra_basic INTERFACE)
target_include_directories(extra_basic INTERFACE include/extra)
target_compile_features(extra_basic INTERFACE cxx_std_20)
target_compile_features(extra_basic INTERFACE c_std_11)

add_library(extra_test_basic INTERFACE)
target_include_directories(extra_test_basic INTERFACE include)
target_compile_features(extra_test_basic INTERFACE cxx_std_20)
target_compile_features(extra_test_basic INTERFACE c_std_11)

add_subdirectory(src)
add_subdirectory(3rd)
add_subdirectory(test)
