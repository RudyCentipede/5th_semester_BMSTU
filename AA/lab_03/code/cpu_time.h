#ifndef CPU_TIME_H
#define CPU_TIME_H

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include "algorithms.h"

#define ITER_NUM 10000

void measure_cpu_time(size_t start_len, size_t stop_len, size_t step);

#endif
