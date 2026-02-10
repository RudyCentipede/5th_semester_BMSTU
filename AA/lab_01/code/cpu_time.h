#ifndef CPU_TIME_H
#define CPU_TIME_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "matrix.h"

#define ITER_NUM 1000

void measure_cpu_time(size_t start_len, size_t stop_len, size_t step);

#endif
