#include "cpu_time.h"
#include "graph_dbscan.h"

static long long get_nanotime() {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (long long)ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

void generate_random_dot(const string &filename, int n, int max_deg,
                         bool directed) {
  ofstream fout(filename);
  if (!fout.is_open()) {
    cerr << "Ошибка создания файла " << filename << endl;
    return;
  }

  fout << (directed ? "digraph G {\n" : "graph G {\n");
  fout << "  node [style=filled];\n";
  for (int i = 0; i < n; i++)
    fout << "  v" << i << ";\n";

  random_device rd;
  mt19937 gen(rd());
  uniform_int_distribution<> deg_dist(0, max_deg - 1);
  uniform_int_distribution<> v_dist(0, n - 1);

  for (int i = 0; i < n; i++) {
    int deg = deg_dist(gen);
    for (int j = 0; j < deg; j++) {
      int to = v_dist(gen);
      if (to == i)
        continue;
      fout << "  v" << i << (directed ? " -> " : " -- ") << "v" << to << ";\n";
    }
  }
  fout << "}\n";
  fout.close();
}

void cpu_time_single_parallel(int M, int minPts, bool directed) {
  cout << "------------------------------------------------------------\n";
  cout << setw(10) << "N" << setw(15) << "Seq(ms)" << setw(20) << "Par(1)(ms)"
       << setw(15) << "Speedup" << "\n";
  cout << "------------------------------------------------------------\n";

  vector<int> sizes = {500, 1000, 1500, 2000, 2500, 3000, 3500, 4000};
  ofstream csv("benchmark_seq_vs_par1.csv");
  csv << "N,Seq_ms,Par1_ms,Speedup\n";

  for (int N : sizes) {
    string fname = "graph_" + to_string(N) + ".dot";
    generate_random_dot(fname, N, 15, directed);

    double seq_avg = 0.0, par_avg = 0.0;
    int repeats = 50;

    for (int r = 0; r < repeats; r++) {
      long long t1 = get_nanotime();
      dbscan(fname, M, minPts, directed, false);
      long long t2 = get_nanotime();
      seq_avg += (double)(t2 - t1) / 1e6;

      long long t3 = get_nanotime();
      dbscan_parallel(fname, M, minPts, 1, directed, false);
      long long t4 = get_nanotime();
      par_avg += (double)(t4 - t3) / 1e6;
    }

    seq_avg /= repeats;
    par_avg /= repeats;
    double speedup = seq_avg / par_avg;

    cout << setw(10) << N << setw(15) << fixed << setprecision(2) << seq_avg
         << setw(20) << fixed << setprecision(2) << par_avg << setw(15) << fixed
         << setprecision(2) << speedup << "\n";

    csv << N << "," << seq_avg << "," << par_avg << "," << speedup << "\n";
  }

  csv.close();
  cout << "------------------------------------------------------------\n";
}

void cpu_time_parallel(int M, int minPts, bool directed) {
  int q = 12;
  vector<int> sizes = {500, 1000, 1500, 2000, 2500, 3000, 3500, 4000};
  vector<int> threads_list;
  for (int k = 1; k <= 8 * q; k *= 2)
    threads_list.push_back(k);
  threads_list.push_back(96);

  cout << "Обнаружено логических ядер: " << q << "\n";
  cout << "-------------------------------------------------------------\n";
  cout << setw(10) << "Threads" << setw(10) << "N" << setw(20) << "Time(ms)"
       << "\n";
  cout << "-------------------------------------------------------------\n";

  ofstream csv("benchmark_scaling_threads_sizes.csv");
  csv << "Threads,N,Time_ms\n";

  for (int threads : threads_list) {
    for (int N : sizes) {
      string fname = "graph_" + to_string(N) + ".dot";
      generate_random_dot(fname, N, 15, directed);

      double avg_ms = 0;
      int repeats = 50;
      for (int r = 0; r < repeats; r++) {
        long long t1 = get_nanotime();
        dbscan_parallel(fname, M, minPts, threads, directed, false);
        long long t2 = get_nanotime();
        avg_ms += (double)(t2 - t1) / 1e6;
      }
      avg_ms /= repeats;

      cout << setw(10) << threads << setw(10) << N << setw(20) << fixed
           << setprecision(2) << avg_ms << "\n";

      csv << threads << "," << N << "," << avg_ms << "\n";
    }
  }

  csv.close();
  cout << "-------------------------------------------------------------\n";
}
