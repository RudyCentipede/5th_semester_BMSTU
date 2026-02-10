#include "graph_dbscan.h"

graph_t load_graph(const string &filename, bool directed, bool verbose) {
  ifstream fin(filename);
  if (!fin.is_open()) {
    cerr << "Ошибка: не удалось открыть " << filename << endl;
    exit(1);
  }

  graph_t g;
  unordered_map<string, int> map;
  string line;
  while (getline(fin, line)) {
    line.erase(remove_if(line.begin(), line.end(), ::isspace), line.end());

    if (line.empty())
      continue;
    if (line[0] == '{' || line[0] == '}' || line[0] == '/' || line[0] == '#')
      continue;
    if (line.rfind("graph", 0) == 0 || line.rfind("digraph", 0) == 0)
      continue;
    if (line.rfind("node[", 0) == 0 || line.rfind("edge[", 0) == 0)
      continue;

    if (line.find("->") == string::npos && line.find("--") == string::npos) {
      string node = line;
      node.erase(remove(node.begin(), node.end(), ';'), node.end());
      if (!node.empty() && !map.count(node)) {
        map[node] = g.names.size();
        g.names.push_back(node);
        g.adj.emplace_back();
      }
      continue;
    }

    string arrow = directed ? "->" : "--";
    size_t pos = line.find(arrow);
    if (pos == string::npos)
      continue;

    string a = line.substr(0, pos);
    string b = line.substr(pos + arrow.size());
    b.erase(remove(b.begin(), b.end(), ';'), b.end());

    if (a.empty() || b.empty())
      continue;

    if (!map.count(a)) {
      map[a] = g.names.size();
      g.names.push_back(a);
      g.adj.emplace_back();
    }
    if (!map.count(b)) {
      map[b] = g.names.size();
      g.names.push_back(b);
      g.adj.emplace_back();
    }

    int u = map[a], v = map[b];
    g.adj[u].push_back(v);
    if (!directed)
      g.adj[v].push_back(u);
  }

  fin.close();

  if (verbose) {
    size_t edges = 0;
    for (auto &v : g.adj)
      edges += v.size();
    cout << "Граф загружен: " << g.adj.size() << " вершин, " << edges
         << " рёбер (" << (directed ? "ориентированный" : "неориентированный")
         << ")\n";
  }

  return g;
}

vector<int> bfs_distances(const graph_t &g, int start, int M) {
  int n = g.adj.size();
  vector<int> dist(n, -1);
  queue<int> q;
  dist[start] = 0;
  q.push(start);
  while (!q.empty()) {
    int v = q.front();
    q.pop();
    if (dist[v] == M)
      continue;
    for (int to : g.adj[v]) {
      if (dist[to] == -1) {
        dist[to] = dist[v] + 1;
        q.push(to);
      }
    }
  }
  return dist;
}

static string color_for_cluster(int cluster_id) {
  static const vector<string> palette = {
      "#1f77b4", "#ff7f0e", "#2ca02c", "#d62728", "#9467bd", "#8c564b",
      "#e377c2", "#7f7f7f", "#bcbd22", "#17becf", "#393b79", "#637939",
      "#8c6d31", "#843c39", "#7b4173", "#3182bd", "#e6550d", "#31a354",
      "#756bb1", "#636363", "#9ecae1", "#ffbb78", "#98df8a", "#ff9896",
      "#c5b0d5", "#c49c94", "#f7b6d2", "#c7c7c7", "#dbdb8d", "#9edae5"};
  if (cluster_id <= 0)
    return "#808080";
  return palette[cluster_id % palette.size()];
}

void save_colored_graph(const graph_t &g, const string &base_filename,
                        bool directed,
                        const vector<int> *cluster_id_ptr = nullptr) {
  string dot_name =
      base_filename.substr(0, base_filename.find_last_of('.')) + "_colored.dot";
  ofstream fout(dot_name);
  if (!fout) {
    cerr << "Ошибка записи файла " << dot_name << endl;
    return;
  }

  fout << (directed ? "digraph G {\n" : "graph G {\n");
  fout << "  node [style=filled, shape=circle, fontsize=10];\n";

  int n = g.adj.size();
  for (int i = 0; i < n; i++) {
    string node = (i < (int)g.names.size()) ? g.names[i] : "v" + to_string(i);
    string color =
        cluster_id_ptr ? color_for_cluster((*cluster_id_ptr)[i]) : "#cccccc";
    fout << "  \"" << node << "\" [fillcolor=\"" << color << "\", label=\""
         << node << "\"];\n";
  }

  string conn = directed ? "->" : "--";
  for (int i = 0; i < n; i++)
    for (int j : g.adj[i])
      if (directed || i < j)
        fout << "  \"" << g.names[i] << "\" " << conn << " \"" << g.names[j]
             << "\";\n";

  fout << "}\n";
  fout.close();

  string svg = dot_name.substr(0, dot_name.find_last_of('.')) + ".svg";
  string cmd = "dot -Tsvg \"" + dot_name + "\" -o \"" + svg + "\"";
  system(cmd.c_str());
}

static void clustering(const graph_t &g, int minPts,
                       const vector<vector<int>> &regions,
                       vector<int> &cluster_id, bool verbose,
                       const string &filename, bool directed) {
  int n = g.adj.size();
  int cluster = 0;

  for (int i = 0; i < n; i++) {
    if (cluster_id[i] != 0)
      continue;
    if (regions[i].size() < minPts) {
      cluster_id[i] = -1;
      continue;
    }

    cluster++;
    cluster_id[i] = cluster;
    deque<int> seeds(regions[i].begin(), regions[i].end());

    while (!seeds.empty()) {
      int cur = seeds.front();
      seeds.pop_front();
      if (cluster_id[cur] == -1)
        cluster_id[cur] = cluster;
      if (cluster_id[cur] != 0)
        continue;
      cluster_id[cur] = cluster;
      if (regions[cur].size() >= minPts)
        for (int nb : regions[cur])
          seeds.push_back(nb);
    }
  }

  if (verbose) {
    unordered_map<int, int> counts;
    for (int cid : cluster_id)
      counts[cid]++;
    cout << "Найдено кластеров: " << cluster << "\n";
    for (int c = 1; c <= cluster; c++)
      cout << "  Кластер " << c << ": " << counts[c] << " вершин\n";
    cout << "  Шум (-1): " << counts[-1] << " вершин\n";
    save_colored_graph(g, filename, directed, &cluster_id);
  }
}

void dbscan(const string &filename, int M, int minPts, bool directed,
            bool verbose) {
  graph_t g = load_graph(filename, directed, verbose);
  int n = g.adj.size();

  vector<vector<int>> regions(n);
  for (int i = 0; i < n; i++) {
    auto dist = bfs_distances(g, i, M);
    for (int j = 0; j < n; j++)
      if (i != j && dist[j] != -1 && dist[j] <= M)
        regions[i].push_back(j);
  }

  vector<int> cluster_id(n, 0);
  clustering(g, minPts, regions, cluster_id, verbose, filename, directed);
}

void dbscan_parallel(const string &filename, int M, int minPts, int threads,
                     bool directed, bool verbose) {
  graph_t g = load_graph(filename, directed, verbose);
  int n = g.adj.size();

  vector<vector<int>> regions(n);

  mutex m;
  int next = 0;
  auto worker = [&]() {
    while (true) {
      int i;
      {
        lock_guard<mutex> guard(m);
        if (next >= n)
          return;
        i = next++;
      }
      auto dist = bfs_distances(g, i, M);
      vector<int> local;
      for (int j = 0; j < n; j++)
        if (i != j && dist[j] != -1 && dist[j] <= M)
          local.push_back(j);
      regions[i].swap(local);
    }
  };

  vector<thread> thrds;
  for (int i = 0; i < threads; i++)
    thrds.emplace_back(worker);
  for (auto &th : thrds)
    th.join();

  vector<int> cluster_id(n, 0);
  clustering(g, minPts, regions, cluster_id, verbose, filename, directed);
}

string dbscan_cli(const graph_t &g, int M, int minPts) {
  int n = g.adj.size();

  vector<vector<int>> regions(n);
  for (int i = 0; i < n; i++) {
    auto dist = bfs_distances(g, i, M);
    for (int j = 0; j < n; j++)
      if (i != j && dist[j] != -1 && dist[j] <= M)
        regions[i].push_back(j);
  }

  vector<int> cluster_id(n, 0);
  int cluster = 0;

  for (int i = 0; i < n; i++) {
    if (cluster_id[i] != 0)
      continue;
    if ((int)regions[i].size() < minPts) {
      cluster_id[i] = -1;
      continue;
    }

    cluster++;
    cluster_id[i] = cluster;
    deque<int> seeds(regions[i].begin(), regions[i].end());

    while (!seeds.empty()) {
      int cur = seeds.front();
      seeds.pop_front();
      if (cluster_id[cur] == -1)
        cluster_id[cur] = cluster;
      if (cluster_id[cur] != 0)
        continue;
      cluster_id[cur] = cluster;
      if ((int)regions[cur].size() >= minPts)
        for (int nb : regions[cur])
          seeds.push_back(nb);
    }
  }

  map<int, vector<int>> clusters;
  vector<int> noise;

  for (int i = 0; i < n; i++) {
    int id = 0;
    try {
      id = stoi(g.names[i]);
    } catch (...) {
      id = i + 1;
    }
    if (cluster_id[i] == -1)
      noise.push_back(id);
    else
      clusters[cluster_id[i]].push_back(id);
  }

  for (auto &kv : clusters)
    sort(kv.second.begin(), kv.second.end());
  sort(noise.begin(), noise.end());

  ostringstream oss;
  oss << "{\"clusters\":[";
  bool firstCluster = true;
  for (auto &kv : clusters) {
    if (!firstCluster)
      oss << ",";
    firstCluster = false;
    oss << "[";
    for (size_t j = 0; j < kv.second.size(); j++) {
      if (j)
        oss << ",";
      oss << kv.second[j];
    }
    oss << "]";
  }
  oss << "],\"noise\":[";
  for (size_t i = 0; i < noise.size(); i++) {
    if (i)
      oss << ",";
    oss << noise[i];
  }
  oss << "]}";
  return oss.str();
}

string dbscan_parallel_cli(const graph_t &g, int M, int minPts, int threads) {
  int n = g.adj.size();

  vector<vector<int>> regions(n);
  mutex lock;
  int next = 0;

  auto worker = [&]() {
    while (true) {
      int i;
      {
        lock_guard<mutex> guard(lock);
        if (next >= n)
          return;
        i = next++;
      }
      auto dist = bfs_distances(g, i, M);
      vector<int> local;
      for (int j = 0; j < n; j++)
        if (i != j && dist[j] != -1 && dist[j] <= M)
          local.push_back(j);
      regions[i].swap(local);
    }
  };

  vector<thread> pool;
  for (int t = 0; t < threads; t++)
    pool.emplace_back(worker);
  for (auto &th : pool)
    th.join();

  vector<int> cluster_id(n, 0);
  int cluster = 0;

  for (int i = 0; i < n; i++) {
    if (cluster_id[i] != 0)
      continue;
    if ((int)regions[i].size() < minPts) {
      cluster_id[i] = -1;
      continue;
    }

    cluster++;
    cluster_id[i] = cluster;
    deque<int> seeds(regions[i].begin(), regions[i].end());

    while (!seeds.empty()) {
      int cur = seeds.front();
      seeds.pop_front();
      if (cluster_id[cur] == -1)
        cluster_id[cur] = cluster;
      if (cluster_id[cur] != 0)
        continue;
      cluster_id[cur] = cluster;
      if ((int)regions[cur].size() >= minPts)
        for (int nb : regions[cur])
          seeds.push_back(nb);
    }
  }

  map<int, vector<int>> clusters;
  vector<int> noise;

  for (int i = 0; i < n; i++) {
    int id = 0;
    try {
      id = stoi(g.names[i]);
    } catch (...) {
      id = i + 1;
    }
    if (cluster_id[i] == -1)
      noise.push_back(id);
    else
      clusters[cluster_id[i]].push_back(id);
  }

  for (auto &kv : clusters)
    sort(kv.second.begin(), kv.second.end());
  sort(noise.begin(), noise.end());

  ostringstream oss;
  oss << "{\"clusters\":[";
  bool firstCluster = true;
  for (auto &kv : clusters) {
    if (!firstCluster)
      oss << ",";
    firstCluster = false;
    oss << "[";
    for (size_t j = 0; j < kv.second.size(); j++) {
      if (j)
        oss << ",";
      oss << kv.second[j];
    }
    oss << "]";
  }
  oss << "],\"noise\":[";
  for (size_t i = 0; i < noise.size(); i++) {
    if (i)
      oss << ",";
    oss << noise[i];
  }
  oss << "]}";
  return oss.str();
}
