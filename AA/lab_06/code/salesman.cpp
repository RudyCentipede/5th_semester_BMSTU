#include "salesman.h"

int n;
double distM[MAX_N][MAX_N];

int bestPathBrute[MAX_N];
double bestLenBrute;

double pheromone[MAX_N][MAX_N];
double heuristicV[MAX_N][MAX_N];

int antPath[NUM_ANTS][MAX_N];
double antLen[NUM_ANTS];

int bestPathACO[MAX_N];
double bestLenACO;

void solve_bruteforce_iterative() {
  int perm[MAX_N];
  for (int i = 0; i < n; ++i)
    perm[i] = i;

  bestLenBrute = INF;

  bool first = true;
  do {
    double len = 0.0;
    bool ok = true;

    for (int i = 0; i < n - 1; ++i) {
      int a = perm[i];
      int b = perm[i + 1];

      if (distM[a][b] >= INF / 2.0) {
        ok = false;
        break;
      }

      len += distM[a][b];

      if (len >= bestLenBrute) {
        ok = false;
        break;
      }
    }

    if (ok) {
      if (first || len < bestLenBrute) {
        first = false;
        bestLenBrute = len;
        memcpy(bestPathBrute, perm, n * sizeof(int));
      }
    }

  } while (std::next_permutation(perm, perm + n));
}

double rnd01() { return (double)rand() / (double)RAND_MAX; }

void init_heuristic() {
  for (int i = 0; i < n; ++i)
    for (int j = 0; j < n; ++j) {
      if (i != j && distM[i][j] < INF / 2.0)
        heuristicV[i][j] = 1.0 / distM[i][j];
      else
        heuristicV[i][j] = 0.0;
    }
}

void init_pheromone() {
  for (int i = 0; i < n; ++i)
    for (int j = 0; j < n; ++j) {
      if (i != j && distM[i][j] < INF / 2.0)
        pheromone[i][j] = 1.0;
      else
        pheromone[i][j] = 0.0;
    }
}

double path_length(int *path) {
  double len = 0.0;
  for (int i = 0; i < n - 1; ++i) {
    double d = distM[path[i]][path[i + 1]];
    if (d >= INF / 2.0)
      return INF;
    len += d;
  }
  return len;
}

void generate_ant_solution(int k) {
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

        double val = pow(tau, BETA) * pow(eta, ALPHA);
        prob[v] = val;
        sumProb += val;
      } else
        prob[v] = 0.0;
    }

    if (sumProb == 0.0) {
      antLen[k] = INF;
      return;
    }

    double r = rnd01() * sumProb;
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

  antLen[k] = path_length(antPath[k]);
}

void run_aco() {
  init_heuristic();
  init_pheromone();
  bestLenACO = INF;

  for (int iter = 0; iter < MAX_ITER; ++iter) {
    for (int k = 0; k < NUM_ANTS; ++k)
      generate_ant_solution(k);

    int bestAntIter = -1;
    double bestLenIter = INF;

    for (int k = 0; k < NUM_ANTS; ++k) {
      if (antLen[k] < bestLenIter) {
        bestLenIter = antLen[k];
        bestAntIter = k;
      }
    }

    if (bestLenIter < bestLenACO) {
      bestLenACO = bestLenIter;
      for (int i = 0; i < n; ++i)
        bestPathACO[i] = antPath[bestAntIter][i];
    }

    for (int i = 0; i < n; ++i)
      for (int j = 0; j < n; ++j)
        pheromone[i][j] *= (1.0 - RHO);

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

    if (bestLenACO < INF / 2.0) {
      double eliteContrib = ELITE_ANTS * QVAL / bestLenACO;
      for (int s = 0; s < n - 1; ++s) {
        int i = bestPathACO[s];
        int j = bestPathACO[s + 1];
        pheromone[i][j] += eliteContrib;
      }
    }
  }
}