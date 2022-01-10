
/**
 * @file jump2_test.cpp
 * @author Yimin Liu (aoumeior@outlook.com)
 * @brief 
 * @version 0.1
 * @date 2022-01-08
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <gtest/gtest.h> // Google Test Framework

#include <vector> // for std::vector

#include "JumpGame2.h" // Solution of problem.

TEST(JumpGame2, test) {
  std::vector<int> nums = {2, 3, 1, 1, 4};
  EXPECT_EQ(2, JumpGame2::Solution(nums));
}