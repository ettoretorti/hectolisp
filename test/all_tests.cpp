#include <gtest/gtest.h>

#include "mem_tests.cpp"

#include "parse_tests.cpp"

#include "print_tests.cpp"

#include "eval_tests.cpp"

int main(int argc, char** argv) {
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
