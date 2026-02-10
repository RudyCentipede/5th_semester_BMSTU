#include "pipeline.h"
#include <iostream>
#include <cstdlib>

int main(int argc, char** argv) {
  // ./app-pipe <filelist.txt> <N> <M_default> <eps_default> <minPts_default> <k> <directed 0|1> <prefix>
  if (argc < 9) {
    std::cerr << "Usage: " << argv[0]
              << " <filelist.txt> <N> <M_default> <eps_default> <minPts_default> <k> <directed 0|1> <prefix>\n";
    return 1;
  }

  PipelineParams p;
  p.filelist_path = argv[1];
  p.N = std::atoi(argv[2]);
  p.M = std::atoi(argv[3]);
  p.eps = std::atof(argv[4]);
  p.minPts = std::atoi(argv[5]);
  p.k = std::atoi(argv[6]);
  p.directed = (std::atoi(argv[7]) != 0);
  p.out_prefix = argv[8];

  return run_pipeline_experiment(p);
}
