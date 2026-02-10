#ifndef CPU_TIME_H
#define CPU_TIME_H

#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <thread>
#include <vector>

using namespace std;

void cpu_time_single_parallel(int M, int minPts, bool directed);
void cpu_time_parallel(int M, int minPts, bool directed);

#endif
