#pragma once

#include <new>

namespace extra::util
{
#ifdef __cpp_lib_hardware_interface_size
constexpr static inline size_t cache_line_size = std::hardware_destructive_interface_size;
#else
constexpr static inline size_t cache_line_size = 64;
#endif

constexpr inline size_t pow_of_2(size_t n)
{
	n--;
	size_t m = 1;
	while (n != 0)
	{
		n >>= 1;
		m <<= 1;
	}
	return m;
}
}