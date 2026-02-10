#ifndef SALESMAN_H
#define SALESMAN_H

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

const int MAX_N = 20;
const double INF = 1e9;

extern int n;
extern double distM[MAX_N][MAX_N];

extern int bestPathBrute[MAX_N];
extern double bestLenBrute;

const int NUM_ANTS = 20;
const int MAX_ITER = 5;

const double ALPHA_BASE = 0.6;
const double ALPHA = ALPHA_BASE;
const double BETA = 1.0 - ALPHA_BASE;
const double RHO = 0.5;
const double QVAL = 100.0;
const int ELITE_ANTS = 5;

extern double pheromone[MAX_N][MAX_N];
extern double heuristicV[MAX_N][MAX_N];

extern int antPath[NUM_ANTS][MAX_N];
extern double antLen[NUM_ANTS];

extern int bestPathACO[MAX_N];
extern double bestLenACO;

void run_aco();
void solve_bruteforce_iterative();

#endif
