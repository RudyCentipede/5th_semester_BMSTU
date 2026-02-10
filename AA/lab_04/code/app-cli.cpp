#include "graph_dbscan.h"

int main(int argc, char *argv[]) {
  if (argc < 6) {
    cerr << "Usage: " << argv[0]
         << " <graph.dot> <M> <eps> <minPts> <threads>\n";
    return 1;
  }

  string filename = argv[1];
  int M = stoi(argv[2]);
  double eps = stod(argv[3]);
  int minPts = stoi(argv[4]);
  int threads = stoi(argv[5]);
  bool directed = true;

  graph_t g = load_graph(filename, directed, false);

  string output;
  if (threads == 0)
    output = dbscan_cli(g, M, minPts);
  else
    output = dbscan_parallel_cli(g, M, minPts, threads);

  cout << output << endl;
  return 0;
}
