/*
 * main.cpp
 *
 *  Created on: Mar 7, 2015
 *      Author: leeopop
 */


#include <gtest/gtest.h>

int main(int argc, char** argv)
{
	testing::InitGoogleTest(&argc, argv);
	int result = RUN_ALL_TESTS();
	EXPECT_TRUE(result == 0);
	return 0;
}
