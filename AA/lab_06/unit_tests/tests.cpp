#include "salesman.h"
#include <gtest/gtest.h>

static void reset_graph(int vertices) {
  n = vertices;
  for (int i = 0; i < n; ++i)
    for (int j = 0; j < n; ++j)
      distM[i][j] = INF;
}

TEST(Bruteforce, ThreeVerticesUniqueOptimal) {
  reset_graph(3);

  distM[0][1] = 1;
  distM[1][2] = 1;

  distM[0][2] = 10;
  distM[1][0] = 10;
  distM[2][1] = 10;
  distM[2][0] = 10;

  solve_bruteforce_iterative();

  EXPECT_NEAR(bestLenBrute, 2.0, 1e-9);

  int expected[3] = {0, 1, 2};
  for (int i = 0; i < 3; ++i)
    EXPECT_EQ(bestPathBrute[i], expected[i]);
}

TEST(Bruteforce, FourVerticesWithForbiddenEdges) {
  reset_graph(4);

  distM[0][1] = 1;
  distM[1][2] = 1;
  distM[2][3] = 1;

  distM[1][0] = 5;
  distM[2][1] = 5;
  distM[3][2] = 5;

  solve_bruteforce_iterative();

  EXPECT_NEAR(bestLenBrute, 3.0, 1e-9);

  int expected[4] = {0, 1, 2, 3};
  for (int i = 0; i < 4; ++i)
    EXPECT_EQ(bestPathBrute[i], expected[i]);
}

TEST(Bruteforce, SingleVertex) {
  reset_graph(1);

  solve_bruteforce_iterative();

  EXPECT_NEAR(bestLenBrute, 0.0, 1e-9);
  EXPECT_EQ(bestPathBrute[0], 0);
}

TEST(Bruteforce, NoHamiltonianPath) {
  reset_graph(3);

  distM[0][1] = 1;
  distM[1][0] = 1;

  solve_bruteforce_iterative();

  EXPECT_GE(bestLenBrute, INF / 2.0);
}
