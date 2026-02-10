#include "algorithms.h"

size_t count_ones_recursive(const int *arr, size_t i) { // 0
  if (arr[i] == 2)                                      // 1
    return 0;                                           // 2

  if (arr[i] == 1)                               // 3
    return 1 + count_ones_recursive(arr, i + 1); // 4

  return count_ones_recursive(arr, i + 1); // 5
}

size_t count_ones_iterative(const int *arr) { // 0
  size_t count = 0;                           // 1
  for (size_t i = 0; arr[i] != 2; i++)        // 2
    if (arr[i] == 1)                          // 3
      count++;                                // 4

  return count; // 5
}