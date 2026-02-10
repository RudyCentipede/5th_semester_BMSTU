#include "algorithms.h"
#include <gtest/gtest.h>
#include <vector>

TEST(IterativeTest, EmptySequence) {
  int sequence[] = {2};
  size_t result = count_ones_iterative(sequence);
  EXPECT_EQ(result, 0);
}

TEST(IterativeTest, AllZeros) {
  int sequence[] = {0, 0, 0, 0, 2};
  size_t result = count_ones_iterative(sequence);
  EXPECT_EQ(result, 0);
}

TEST(IterativeTest, AllOnes) {
  int sequence[] = {1, 1, 1, 1, 2};
  size_t result = count_ones_iterative(sequence);
  EXPECT_EQ(result, 4);
}

TEST(IterativeTest, Alternating) {
  int sequence[] = {1, 0, 1, 0, 1, 0, 1, 0, 2};
  size_t result = count_ones_iterative(sequence);
  EXPECT_EQ(result, 4);
}

TEST(IterativeTest, SingleOne) {
  int sequence[] = {1, 2};
  size_t result = count_ones_iterative(sequence);
  EXPECT_EQ(result, 1);
}

TEST(IterativeTest, SingleZero) {
  int sequence[] = {0, 2};
  size_t result = count_ones_iterative(sequence);
  EXPECT_EQ(result, 0);
}

TEST(RecursiveTest, EmptySequence) {
  int sequence[] = {2};
  size_t result = count_ones_recursive(sequence, 0);
  EXPECT_EQ(result, 0);
}

TEST(RecursiveTest, AllZeros) {
  int sequence[] = {0, 0, 0, 0, 2};
  size_t result = count_ones_recursive(sequence, 0);
  EXPECT_EQ(result, 0);
}

TEST(RecursiveTest, AllOnes) {
  int sequence[] = {1, 1, 1, 1, 2};
  size_t result = count_ones_recursive(sequence, 0);
  EXPECT_EQ(result, 4);
}

TEST(RecursiveTest, SingleOne) {
  int sequence[] = {1, 2};
  size_t result = count_ones_recursive(sequence, 0);
  EXPECT_EQ(result, 1);
}

TEST(RecursiveTest, SingleZero) {
  int sequence[] = {0, 2};
  size_t result = count_ones_recursive(sequence, 0);
  EXPECT_EQ(result, 0);
}

TEST(RecursiveTest, Alternating) {
  int sequence[] = {1, 0, 1, 0, 1, 0, 1, 0, 2};
  size_t result = count_ones_recursive(sequence, 0);
  EXPECT_EQ(result, 4);
}