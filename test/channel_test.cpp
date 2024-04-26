#include "gtest/gtest.h"
#include "extra/Channel.h"

class Order
{
public:
	long id;
	char name[16];
	long volume;
	long price;
	long timestamp;
	struct {
		int side : 1;
		int position: 1;
	};
};

constexpr static inline size_t length = 1024 * 1024;

TEST(Shm, T0)
{
}
