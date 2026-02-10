#include "salesman.h"

void init_example_graph() {
  n = 7;

  for (int i = 0; i < n; ++i)
    for (int j = 0; j < n; ++j)
      distM[i][j] = INF;

  distM[0][1] = 10;
  distM[1][0] = 14;

  distM[0][2] = 18;
  distM[2][0] = 20;

  distM[1][2] = 11;
  distM[2][1] = 13;

  distM[1][4] = 12;
  distM[4][1] = 16;

  distM[2][3] = 7;
  distM[3][2] = 9;

  distM[2][5] = 20;
  distM[5][2] = 15;

  distM[3][4] = 9;
  distM[4][3] = 11;

  distM[3][6] = 14;
  distM[6][3] = 14;

  distM[4][5] = 6;
  distM[5][4] = 8;

  distM[4][6] = 15;

  distM[5][6] = 9;
  distM[6][5] = 10;
}

void print_matrix() {
  printf("Матрица времен перелетов (INF = нет ребра):\n");
  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < n; ++j) {
      if (distM[i][j] >= INF / 2.0)
        printf("  INF ");
      else
        printf("%5.1f ", distM[i][j]);
    }
    printf("\n");
  }
  printf("\n");
}

void print_path(const int *path) {
  for (int i = 0; i < n; ++i)
    printf("%d ", path[i]);
  printf("\n");
}

int main() {
  srand((unsigned)time(NULL));

  init_example_graph();
  print_matrix();

  solve_bruteforce_iterative();
  printf("Полный перебор (открытый маршрут, без возврата):\n");
  printf("Лучшая длина: %.3f\nМаршрут: ", bestLenBrute);
  print_path(bestPathBrute);
  printf("\n");

  run_aco();
  printf("Муравьиный алгоритм (открытый маршрут, элитные муравьи):\n");
  printf("Лучшая длина: %.3f\nМаршрут: ", bestLenACO);
  print_path(bestPathACO);
  printf("\n");

  return 0;
}
