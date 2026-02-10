#include "pipeline.h"

#include <deque>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <iostream>
#include <cstdio>

static long long get_nanotime() {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (long long)ts.tv_sec * 1000000000LL + (long long)ts.tv_nsec;
}

static double ns_to_us(long long ns) { return (double)ns / 1e3; }


static std::string fmt_duration_ns(long long ns) {
  double ms = (double)ns / 1e6;
  char buf[64];
  if (ms < 1000.0) std::snprintf(buf, sizeof(buf), "%.3f ms", ms);
  else {
    double s = ms / 1000.0;
    if (s < 60.0) std::snprintf(buf, sizeof(buf), "%.3f s", s);
    else {
      int m = (int)(s / 60.0);
      double rem = s - 60.0 * m;
      std::snprintf(buf, sizeof(buf), "%d min %.3f s", m, rem);
    }
  }
  return std::string(buf);
}

static std::string base_name_only(const std::string& path) {
  size_t p1 = path.find_last_of("/\\");
  std::string name = (p1 == std::string::npos) ? path : path.substr(p1 + 1);
  size_t p2 = name.find_last_of('.');
  if (p2 == std::string::npos) return name;
  return name.substr(0, p2);
}


static std::string dbscan_to_json(const graph_t& g, int M, int minPts, int threads) {
  int n = (int)g.adj.size();
  std::vector<std::vector<int>> regions(n);

  if (threads <= 0) {
    for (int i = 0; i < n; ++i) {
      auto dist = bfs_distances(g, i, M);
      for (int j = 0; j < n; ++j)
        if (i != j && dist[j] != -1 && dist[j] <= M)
          regions[i].push_back(j);
    }
  } else {
    std::mutex lock;
    int next = 0;

    auto worker = [&]() {
      while (true) {
        int i;
        {
          std::lock_guard<std::mutex> guard(lock);
          if (next >= n) return;
          i = next++;
        }
        auto dist = bfs_distances(g, i, M);
        std::vector<int> local;
        for (int j = 0; j < n; ++j)
          if (i != j && dist[j] != -1 && dist[j] <= M)
            local.push_back(j);
        regions[i].swap(local);
      }
    };

    std::vector<std::thread> th;
    th.reserve((size_t)threads);
    for (int t = 0; t < threads; ++t)
      th.emplace_back(worker);
    for (auto& x : th)
      x.join();
  }

  std::vector<int> cluster_id(n, 0);
  int cluster = 0;

  for (int i = 0; i < n; ++i) {
    if (cluster_id[i] != 0) continue;
    if ((int)regions[i].size() < minPts) {
      cluster_id[i] = -1;
      continue;
    }
    ++cluster;
    cluster_id[i] = cluster;

    std::deque<int> seeds(regions[i].begin(), regions[i].end());
    while (!seeds.empty()) {
      int cur = seeds.front();
      seeds.pop_front();

      if (cluster_id[cur] == -1) cluster_id[cur] = cluster;
      if (cluster_id[cur] != 0) continue;

      cluster_id[cur] = cluster;
      if ((int)regions[cur].size() >= minPts) {
        for (int nb : regions[cur]) seeds.push_back(nb);
      }
    }
  }

  std::map<int, std::vector<int>> clusters;
  std::vector<int> noise;

  for (int i = 0; i < n; ++i) {
    int id_num = 0;
    try { id_num = std::stoi(g.names[i]); }
    catch (...) { id_num = i + 1; }

    if (cluster_id[i] == -1) noise.push_back(id_num);
    else clusters[cluster_id[i]].push_back(id_num);
  }

  for (auto& kv : clusters) std::sort(kv.second.begin(), kv.second.end());
  std::sort(noise.begin(), noise.end());

  std::ostringstream oss;
  oss << "{\"clusters\":[";
  bool firstC = true;
  for (auto& kv : clusters) {
    if (!firstC) oss << ",";
    firstC = false;
    oss << "[";
    for (size_t j = 0; j < kv.second.size(); ++j) {
      if (j) oss << ",";
      oss << kv.second[j];
    }
    oss << "]";
  }
  oss << "],\"noise\":[";
  for (size_t i = 0; i < noise.size(); ++i) {
    if (i) oss << ",";
    oss << noise[i];
  }
  oss << "]}";
  return oss.str();
}


static void write_answer_file(const Request& r) {
  std::string base = base_name_only(r.filename);
  std::ostringstream name;
  name << r.out_prefix << r.id << "_" << base << ".json";

  std::ofstream fout(name.str());
  if (!fout) return;
  fout << r.json_result;
  fout.close();
}


static bool read_requests_from_filelist(const PipelineParams& p, std::vector<Request*>& out) {
  std::ifstream fin(p.filelist_path);
  if (!fin.is_open()) return false;

  std::vector<std::string> lines;
  std::string line;
  while (std::getline(fin, line)) {

    while (!line.empty() && (line.back()=='\r' || line.back()=='\n' || line.back()==' ' || line.back()=='\t'))
      line.pop_back();
    size_t i = 0;
    while (i < line.size() && (line[i]==' ' || line[i]=='\t')) i++;
    if (i > 0) line = line.substr(i);

    if (line.empty()) continue;
    if (line[0] == '#') continue;
    lines.push_back(line);
  }
  fin.close();

  if ((int)lines.size() < p.N)
    return false;


  out.reserve((size_t)p.N);
  for (int idx = 0; idx < p.N; ++idx) {
    std::istringstream ss(lines[idx]);
    std::string fname;
    ss >> fname;
    if (fname.empty()) return false;

    int M = p.M;
    double eps = p.eps;
    int minPts = p.minPts;


    if (!(ss >> M)) { M = p.M; ss.clear(); }
    if (!(ss >> eps)) { eps = p.eps; ss.clear(); }
    if (!(ss >> minPts)) { minPts = p.minPts; ss.clear(); }

    Request* r = new Request();
    r->id = idx + 1;
    r->filename = fname;
    r->M = M;
    r->eps = eps;
    r->minPts = minPts;
    r->directed = p.directed;
    r->out_prefix = p.out_prefix;

    for (int k = 0; k < 3; ++k) { r->t_start[k] = 0; r->t_end[k] = 0; }
    out.push_back(r);
  }
  return true;
}


static long long run_sequential(std::vector<Request*>& reqs) {
  long long t0 = get_nanotime();

  for (Request* r : reqs) {

    r->t_start[0] = get_nanotime();
    r->g = load_graph(r->filename, r->directed, false);
    r->t_end[0] = get_nanotime();


    r->t_start[1] = get_nanotime();
    r->json_result = dbscan_to_json(r->g, r->M, r->minPts, 0);
    r->t_end[1] = get_nanotime();


    r->t_start[2] = get_nanotime();
    write_answer_file(*r);
    r->t_end[2] = get_nanotime();
  }

  long long t1 = get_nanotime();
  return t1 - t0;
}


static long long run_pipeline(std::vector<Request*>& reqs, int k) {
  int threads = k - 3;
  if (threads < 0) threads = 0;

  BlockingQueue<Request*> Q1, Q2, Q3;
  for (Request* r : reqs) Q1.push(r);
  Q1.close();

  auto ou1 = [&]() {
    Request* r;
    while (Q1.pop(r)) {
      r->t_start[0] = get_nanotime();
      r->g = load_graph(r->filename, r->directed, false);
      r->t_end[0] = get_nanotime();
      Q2.push(r);
    }
    Q2.close();
  };

  auto ou2 = [&]() {
    Request* r;
    while (Q2.pop(r)) {
      r->t_start[1] = get_nanotime();
      r->json_result = dbscan_to_json(r->g, r->M, r->minPts, threads);
      r->t_end[1] = get_nanotime();
      Q3.push(r);
    }
    Q3.close();
  };

  auto ou3 = [&]() {
    Request* r;
    while (Q3.pop(r)) {
      r->t_start[2] = get_nanotime();
      write_answer_file(*r);
      r->t_end[2] = get_nanotime();
    }
  };
  long long t0 = get_nanotime();
  std::thread t1(ou1);
  std::thread t2(ou2);
  std::thread t3(ou3);
  t1.join();
  t2.join();
  t3.join();
  long long t1_end = get_nanotime();
  return t1_end - t0;
}

static void print_log_sorted(const std::vector<Request*>& reqs) {
  std::vector<Event> ev;
  ev.reserve(reqs.size() * 6);

  for (const Request* r : reqs) {
    ev.push_back({r->t_start[0], r->id, 1, "START"});
    ev.push_back({r->t_end[0],   r->id, 1, "END"});
    ev.push_back({r->t_start[1], r->id, 2, "START"});
    ev.push_back({r->t_end[1],   r->id, 2, "END"});
    ev.push_back({r->t_start[2], r->id, 3, "START"});
    ev.push_back({r->t_end[2],   r->id, 3, "END"});
  }

  std::sort(ev.begin(), ev.end(), [](const Event& a, const Event& b) {
    if (a.t != b.t) return a.t < b.t;
    if (a.req_id != b.req_id) return a.req_id < b.req_id;
    if (a.ou_id != b.ou_id) return a.ou_id < b.ou_id;
    return a.type[0] < b.type[0];
  });

  long long t0 = ev.empty() ? 0 : ev.front().t;

  std::cout.setf(std::ios::fixed);
  std::cout.precision(3);

  for (const auto& e : ev) {
    double dt_us = ns_to_us(e.t - t0);
    std::cout << dt_us << "us "
              << e.type
              << " req=" << e.req_id
              << " ou=" << e.ou_id
              << "\n";
  }
}

int run_pipeline_experiment(const PipelineParams& p) {
  std::vector<Request*> reqs;

  if (!read_requests_from_filelist(p, reqs)) {
    std::cout << "ERROR: filelist must contain at least N valid lines.\n";
    std::cout << "filelist=" << p.filelist_path << " N=" << p.N << "\n";
    return 1;
  }


  long long t_seq = run_sequential(reqs);
  long long t_pipe = run_pipeline(reqs, p.k);

  std::cout << "\n=== TIME ===\n";
  std::cout << "sequential=" << fmt_duration_ns(t_seq) << "\n";
  std::cout << "pipeline=" << fmt_duration_ns(t_pipe) << "\n";
  if (t_pipe > 0) {
    double speedup = (double)t_seq / (double)t_pipe;
    std::cout << "speedup=" << speedup << "\n";
  }

  std::cout << "\n=== LOG ===\n";
  print_log_sorted(reqs);

  for (Request* r : reqs) delete r;
  return 0;
}
