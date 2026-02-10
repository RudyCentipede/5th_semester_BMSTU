#include "algorithms.h"
#include "cpu_time.h"
#include <stdio.h>

#define OK 0
#define CHOICE_ERROR 1

#define MAX_SEQUENCE 10000

int main(void) {
  int choice;
  int sequence[MAX_SEQUENCE];
  int num;
  size_t count = 0;

  printf("----------------Меню----------------------\n");
  printf("1) Рекурсивный алгоритм\n");
  printf("2) Нерекурсивный алгоритм\n");
  printf("3) Выполнить замеры процессорного времени\n");
  printf("0) Выйти\n");

  if (scanf("%d", &choice) != 1 || choice < 0 || choice > 3) {
    printf("Ошибка! Выберите один из пунктов меню.\n");
    return CHOICE_ERROR;
  }

  if (choice == 1 || choice == 2) {
    printf("Введите последовательность, состоящую из нулей и единиц, "
           "заканчивающуюся числом 2:\n");
    count = 0;
    while (scanf("%d", &num) == 1 && num != 2 && count < MAX_SEQUENCE) {
      sequence[count++] = num;
    }
    sequence[count] = 2;
  }

  switch (choice) {
  case 1:
    printf("Количество единиц в последовательности: %zu\n",
           count_ones_recursive(sequence, 0));
    break;

  case 2:
    printf("Количество единиц в последовательности: %zu\n",
           count_ones_iterative(sequence));
    break;

  case 3:
    printf("Замер процессорного времени...\n");
    measure_cpu_time(1000, 10000, 1000);
    break;
  }

  return OK;
}