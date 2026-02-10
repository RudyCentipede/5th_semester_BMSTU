#include "graph_dbscan.h"
#include <cstdio>
#include <fstream>
#include <gtest/gtest.h>
#include <string>

using namespace std;

static string create_temp_dot(const string &content) {
  string filename = "tmp_test_graph.dot";
  ofstream fout(filename);
  fout << content;
  fout.close();
  return filename;
}

TEST(DBSCAN_CLI, SimpleTwoClusters) {
  string dot = R"(digraph G {
        1 -> 2;
        2 -> 3;
        3 -> 4;
        5 -> 6;
        6 -> 7;
        8;
    })";

  string filename = create_temp_dot(dot);
  graph_t g = load_graph(filename, true, false);

  string result = dbscan_cli(g, 2, 2);

  EXPECT_NE(result.find("\"clusters\":[[1,2,3,4],[5,6,7]]"), string::npos);
  EXPECT_NE(result.find("\"noise\":[8]"), string::npos);
}

TEST(DBSCAN_CLI, AllNoise) {
  string dot = R"(digraph G {
        1;
        2;
        3;
        4;
    })";

  string filename = create_temp_dot(dot);
  graph_t g = load_graph(filename, true, false);

  string result = dbscan_cli(g, 1, 2);
  EXPECT_NE(result.find("\"clusters\":[]"), string::npos);
  EXPECT_NE(result.find("\"noise\":[1,2,3,4]"), string::npos);
}

TEST(DBSCAN_CLI, SingleCluster) {
  string dot = R"(digraph G {
        1 -> 2;
        2 -> 3;
        3 -> 4;
        4 -> 5;
        5 -> 1;
    })";

  string filename = create_temp_dot(dot);
  graph_t g = load_graph(filename, true, false);

  string result = dbscan_cli(g, 2, 2);
  EXPECT_NE(result.find("\"clusters\":[[1,2,3,4,5]]"), string::npos);
  EXPECT_NE(result.find("\"noise\":[]"), string::npos);
}

TEST(DBSCAN_CLI, ParallelMatchesSequential) {
  string dot = R"(digraph G {
        1 -> 2;
        2 -> 3;
        3 -> 4;
        5 -> 6;
        6 -> 7;
        8;
    })";

  string filename = create_temp_dot(dot);
  graph_t g = load_graph(filename, true, false);

  string seq = dbscan_cli(g, 2, 2);
  string par = dbscan_parallel_cli(g, 2, 2, 4);

  EXPECT_EQ(seq, par);
}

TEST(DBSCAN_CLI, ThreadsZeroSequential) {
  string dot = R"(digraph G {
        1 -> 2;
        2 -> 3;
        3 -> 4;
    })";

  string filename = create_temp_dot(dot);
  graph_t g = load_graph(filename, true, false);

  string seq = dbscan_cli(g, 1, 2);
  string par = dbscan_parallel_cli(g, 1, 2, 0);

  EXPECT_EQ(seq, par);
}
