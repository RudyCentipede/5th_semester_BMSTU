#pragma once
#include <string>

#include "graph_dbscan.h"

struct PipelineParams {
  std::string filelist_path;
  int N;
  int M;
  double eps;
  int minPts;
  int k;
  bool directed;
  std::string out_prefix;
};

struct Request {
  int id;
  std::string filename;

  int M;
  double eps;
  int minPts;

  bool directed;
  std::string out_prefix;

  graph_t g;

  std::string json_result;

  long long t_start[3];
  long long t_end[3];
};

struct Event {
  long long t;
  int req_id;
  int ou_id;
  const char* type;
};

template <typename T>
struct BlockingQueue {
  std::deque<T> q;
  std::mutex m;
  std::condition_variable cv;
  bool closed = false;

  void push(const T& v) {
    {
      std::lock_guard<std::mutex> g(m);
      q.push_back(v);
    }
    cv.notify_one();
  }

  bool pop(T& out) {
    std::unique_lock<std::mutex> lk(m);

    while (q.empty() && !closed) {
      cv.wait(lk);
    }

    if (q.empty() && closed)
      return false;

    out = q.front();
    q.pop_front();
    return true;
  }

  void close() {
    {
      std::lock_guard<std::mutex> g(m);
      closed = true;
    }
    cv.notify_all();
  }
};

int run_pipeline_experiment(const PipelineParams& p);
