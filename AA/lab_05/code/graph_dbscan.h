#ifndef GRAPH_DBSCAN_H
#define GRAPH_DBSCAN_H

#include <algorithm>
#include <atomic>
#include <bits/stdc++.h>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <mutex>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

using namespace std;

struct graph_t {
  vector<vector<int>> adj;
  vector<string> names;
};

void dbscan(const std::string &filename, int M, int minPts, bool directed,
            bool verbose);
void dbscan_parallel(const std::string &filename, int M, int minPts,
                     int threads, bool directed, bool verbose);
string dbscan_cli(const graph_t &g, int M, int minPts);
string dbscan_parallel_cli(const graph_t &g, int M, int minPts, int threads);
graph_t load_graph(const string &filename, bool directed, bool verbose);
vector<int> bfs_distances(const graph_t &g, int start, int M);

#endif
