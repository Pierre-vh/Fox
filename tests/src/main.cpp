#include "gtest/gtest.h"

int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	auto code = RUN_ALL_TESTS();
	std::cin.get();
	return code;
}

TEST(AnotherTest, FirstTest)
{
	EXPECT_EQ(30, 30);
}