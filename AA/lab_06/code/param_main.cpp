#include "salesman.h"

const int D_NL_N = 10;
double D_NL[10][10] = {{0, 20, 45, 60, 75, 40, 90, 120, 110, 185},
                       {20, 0, 30, 45, 60, 55, 95, 125, 115, 200},
                       {45, 30, 0, 25, 35, 50, 80, 110, 105, 210},
                       {60, 45, 25, 0, 25, 60, 90, 115, 120, 220},
                       {75, 60, 35, 25, 0, 55, 80, 100, 105, 225},
                       {40, 55, 50, 60, 55, 0, 50, 75, 65, 180},
                       {90, 95, 80, 90, 80, 50, 0, 35, 55, 195},
                       {120, 125, 110, 115, 100, 75, 35, 0, 60, 215},
                       {110, 115, 105, 120, 105, 65, 55, 60, 0, 170},
                       {185, 200, 210, 220, 225, 180, 195, 215, 170, 0}};

const int D_CH_N = 10;
double D_CH[10][10] = {{0, 48, 128, INF, 208, INF, INF, INF, INF, INF},
                       {72, 0, 64, 128, 168, 112, INF, INF, INF, INF},
                       {192, 96, 0, 72, 96, 80, INF, INF, INF, 60},
                       {INF, 192, 108, 0, 64, 88, 104, INF, INF, INF},
                       {312, 252, 144, 96, 0, 60, 72, 96, INF, INF},
                       {INF, 168, 120, 132, 40, 0, INF, 88, 120, 84},
                       {INF, INF, INF, 156, 108, INF, 0, 85, INF, INF},
                       {INF, INF, INF, INF, 144, 132, 85, 0, 150, INF},
                       {INF, INF, INF, INF, INF, 180, INF, 150, 0, INF},
                       {INF, INF, 60, INF, INF, 56, INF, INF, INF, 0}};

const int D_AT_N = 10;
double D_AT[10][10] = {{0, 72, INF, INF, INF, INF, 200, INF, INF, 60},
                       {48, 0, 144, INF, INF, INF, INF, INF, INF, INF},
                       {INF, 96, 0, 156, INF, INF, 152, 210, INF, INF},
                       {INF, INF, 104, 0, 168, INF, 176, 160, INF, INF},
                       {INF, INF, INF, 112, 0, 216, INF, 184, INF, INF},
                       {INF, INF, INF, INF, 144, 0, INF, INF, INF, INF},
                       {200, INF, 228, 264, INF, INF, 0, 168, INF, 128},
                       {INF, INF, 210, 240, 276, INF, 112, 0, 72, 144},
                       {INF, INF, INF, INF, INF, INF, INF, 48, 0, INF},
                       {60, INF, INF, INF, INF, INF, 192, 216, INF, 0}};

struct Stats {
  double maxDev;
  double avgDev;
  double medDev;
};

double rnd01_param() { return (double)rand() / (double)RAND_MAX; }

void init_heuristic_param() {
  for (int i = 0; i < n; ++i)
    for (int j = 0; j < n; ++j) {
      if (i != j && distM[i][j] < INF / 2.0)
        heuristicV[i][j] = 1.0 / distM[i][j];
      else
        heuristicV[i][j] = 0.0;
    }
}

void init_pheromone_param() {
  for (int i = 0; i < n; ++i)
    for (int j = 0; j < n; ++j) {
      if (i != j && distM[i][j] < INF / 2.0)
        pheromone[i][j] = 1.0;
      else
        pheromone[i][j] = 0.0;
    }
}

double path_length_param(int *path) {
  double len = 0.0;
  for (int i = 0; i < n - 1; ++i) {
    double d = distM[path[i]][path[i + 1]];
    if (d >= INF / 2.0)
      return INF;
    len += d;
  }
  return len;
}

void generate_ant_solution_param(int k, double alpha, double beta) {
  bool visited[MAX_N];
  for (int i = 0; i < n; ++i)
    visited[i] = false;

  int current = rand() % n;
  antPath[k][0] = current;
  visited[current] = true;

  for (int step = 1; step < n; ++step) {
    double prob[MAX_N];
    double sumProb = 0.0;

    for (int v = 0; v < n; ++v) {
      if (!visited[v] && distM[current][v] < INF / 2.0) {
        double tau = pheromone[current][v];
        double eta = heuristicV[current][v];
        double val = pow(tau, beta) * pow(eta, alpha);
        prob[v] = val;
        sumProb += val;
      } else
        prob[v] = 0.0;
    }

    if (sumProb == 0.0) {
      antLen[k] = INF;
      return;
    }

    double r = rnd01_param() * sumProb;
    double cum = 0.0;
    int next = -1;

    for (int v = 0; v < n; ++v) {
      cum += prob[v];
      if (cum >= r) {
        next = v;
        break;
      }
    }

    if (next == -1) {
      for (int v = 0; v < n; ++v)
        if (prob[v] > 0.0) {
          next = v;
          break;
        }
    }

    antPath[k][step] = next;
    visited[next] = true;
    current = next;
  }

  antLen[k] = path_length_param(antPath[k]);
}

double run_aco_param(int tmax, double alpha, double rho) {
  double beta = 1.0 - alpha;

  init_heuristic_param();
  init_pheromone_param();

  double bestLen = INF;
  int bestPathLocal[MAX_N];

  for (int iter = 0; iter < tmax; ++iter) {
    for (int k = 0; k < NUM_ANTS; ++k)
      generate_ant_solution_param(k, alpha, beta);

    int bestAntIter = -1;
    double bestLenIter = INF;
    for (int k = 0; k < NUM_ANTS; ++k) {
      if (antLen[k] < bestLenIter) {
        bestLenIter = antLen[k];
        bestAntIter = k;
      }
    }

    if (bestLenIter < bestLen) {
      bestLen = bestLenIter;
      for (int i = 0; i < n; ++i)
        bestPathLocal[i] = antPath[bestAntIter][i];
    }

    for (int i = 0; i < n; ++i)
      for (int j = 0; j < n; ++j)
        pheromone[i][j] *= (1.0 - rho);

    for (int k = 0; k < NUM_ANTS; ++k) {
      if (antLen[k] >= INF / 2.0)
        continue;

      double contrib = QVAL / antLen[k];
      for (int s = 0; s < n - 1; ++s) {
        int i = antPath[k][s];
        int j = antPath[k][s + 1];
        pheromone[i][j] += contrib;
      }
    }

    if (bestLen < INF / 2.0) {
      double eliteContrib = ELITE_ANTS * QVAL / bestLen;
      for (int s = 0; s < n - 1; ++s) {
        int i = bestPathLocal[s];
        int j = bestPathLocal[s + 1];
        pheromone[i][j] += eliteContrib;
      }
    }
  }

  return bestLen;
}

void load_graph(double src[10][10], int size) {
  n = size;
  for (int i = 0; i < n; ++i)
    for (int j = 0; j < n; ++j)
      distM[i][j] = src[i][j];
}

void compute_stats_for_graph(double src[10][10], int size, double optimalLen,
                             int tmax, double alpha, double rho, int runs,
                             Stats *out) {
  double devs[100];

  for (int r = 0; r < runs; ++r) {
    load_graph(src, size);
    double len = run_aco_param(tmax, alpha, rho);
    devs[r] = len - optimalLen;
  }

  double maxDev = devs[0];
  double sum = 0.0;
  for (int r = 0; r < runs; ++r) {
    if (devs[r] > maxDev)
      maxDev = devs[r];
    sum += devs[r];
  }
  double avgDev = sum / runs;

  double tmp[100];
  for (int r = 0; r < runs; ++r)
    tmp[r] = devs[r];
  std::sort(tmp, tmp + runs);
  double medDev;
  if (runs % 2 == 1)
    medDev = tmp[runs / 2];
  else
    medDev = 0.5 * (tmp[runs / 2 - 1] + tmp[runs / 2]);

  out->maxDev = maxDev;
  out->avgDev = avgDev;
  out->medDev = medDev;
}

int main() {
  srand((unsigned)time(NULL));

  double optNL, optCH, optAT;

  load_graph(D_NL, D_NL_N);
  solve_bruteforce_iterative();
  optNL = bestLenBrute;

  load_graph(D_CH, D_CH_N);
  solve_bruteforce_iterative();
  optCH = bestLenBrute;

  load_graph(D_AT, D_AT_N);
  solve_bruteforce_iterative();
  optAT = bestLenBrute;

  printf("Optimal lengths (bruteforce): NL=%.3f, CH=%.3f, AT=%.3f\n\n", optNL,
         optCH, optAT);

  int tmaxVals[] = {10, 50, 100};
  double alphaVals[] = {0.2, 0.5, 1};
  double rhoVals[] = {0.1, 0.2, 0.4, 0.8, 1};
  int numT = 3, numA = 3, numR = 5;
  int runs = 10;

  printf("tmax alpha rho | "
         "G1_max G1_avg G1_med | "
         "G2_max G2_avg G2_med | "
         "G3_max G3_avg G3_med\n");

  for (int it = 0; it < numT; ++it) {
    for (int ia = 0; ia < numA; ++ia) {
      for (int ir = 0; ir < numR; ++ir) {
        int tmax = tmaxVals[it];
        double alpha = alphaVals[ia];
        double rho = rhoVals[ir];

        Stats s1, s2, s3;
        compute_stats_for_graph(D_NL, D_NL_N, optNL, tmax, alpha, rho, runs,
                                &s1);
        compute_stats_for_graph(D_CH, D_CH_N, optCH, tmax, alpha, rho, runs,
                                &s2);
        compute_stats_for_graph(D_AT, D_AT_N, optAT, tmax, alpha, rho, runs,
                                &s3);

        printf("%3d %5.2f %5.2f | "
               "%7.1f %7.1f %7.1f | "
               "%7.1f %7.1f %7.1f | "
               "%7.1f %7.1f %7.1f\n",
               tmax, alpha, rho, s1.maxDev, s1.avgDev, s1.medDev, s2.maxDev,
               s2.avgDev, s2.medDev, s3.maxDev, s3.avgDev, s3.medDev);
      }
    }
  }

  return 0;
}
