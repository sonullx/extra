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
	int length = 10;
	extra::kernel::Channel<Order> chan("chunlin", 0, length + 1);
	bool succ = chan.create();
	EXPECT_EQ(succ, true);

	for(int i=0;i<length;i++){
		std::cout << "write iterator " << i << std::endl;
		auto it = chan.write_iterator();
		it->id = i;
		std::cout << "write iterator id " << it->id << std::endl;
		strcpy(it->name, "hello");
		it->volume = i;
		it->price = i;
	}

	auto it_r = chan.read_iterator();
	int expected_id = 0; 
	while(it_r.next()) {
		std::cout << "read result " << it_r->id << std::endl;
		EXPECT_EQ(it_r->id, expected_id);
		expected_id += 1;
	}
	EXPECT_EQ(length, expected_id);
}
