#include <gtest/gtest.h>

int add(int a, int b)
{
	return a + b;
}

TEST(AdditionTest, HandlesPositiveInput)
{
	EXPECT_EQ(5, add(2, 3));
}

TEST(AdditionTest, HandlesNegativeInput)
{
	EXPECT_EQ(-1, add(-2, 1));
	EXPECT_EQ(-3, add(-2, -1));
}
