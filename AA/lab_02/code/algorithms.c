#include "algorithms.h"

size_t count_ones_recursive(const int *arr, size_t i) {
  if (arr[i] == 2)
    return 0;

  if (arr[i] == 1)
    return 1 + count_ones_recursive(arr, i + 1);

  return count_ones_recursive(arr, i + 1);
}

size_t count_ones_iterative(const int *arr) {
  size_t count = 0;
  for (size_t i = 0; arr[i] != 2; i++)
    if (arr[i] == 1)
      count++;

  return count;
}