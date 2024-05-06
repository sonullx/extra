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

constexpr static inline size_t capacity = 1024 * 1024;
using namespace extra::kernel;

TEST(Channel, T0)
{
	using CO = Channel<Order>;
	std::string name = "T0_Order";
	unsigned long version = 0x10101010;

	CO holder{name, version, capacity};
	EXPECT_TRUE(holder.create());

	CO reader{name, version, capacity};
	EXPECT_FALSE(reader.create());
	EXPECT_TRUE(reader.attach());
}
