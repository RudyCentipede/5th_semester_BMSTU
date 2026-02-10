import os
import re
import json
import csv
import subprocess
from datetime import datetime

NS_RE = re.compile(r"^sequential=(.+)$", re.MULTILINE)
NP_RE = re.compile(r"^pipeline=(.+)$", re.MULTILINE)
SP_RE = re.compile(r"^speedup=([0-9.]+)$", re.MULTILINE)


def parse_human_time_to_ms(s: str) -> float:
    s = s.strip()
    m = re.match(r"^([0-9.]+)\s*ms$", s)
    if m:
        return float(m.group(1))
    m = re.match(r"^([0-9.]+)\s*s$", s)
    if m:
        return float(m.group(1)) * 1000.0
    m = re.match(r"^(\d+)\s*min\s*([0-9.]+)\s*s$", s)
    if m:
        minutes = int(m.group(1))
        sec = float(m.group(2))
        return (minutes * 60.0 + sec) * 1000.0

    return float("nan")


def run_once(exe: str, filelist: str, N: int, M: int, eps: float, minPts: int, k: int, directed: int, prefix: str):
    cmd = [exe, filelist, str(N), str(M), str(eps), str(minPts), str(k), str(directed), prefix]
    p = subprocess.run(cmd, capture_output=True, text=True)
    out = p.stdout
    if p.returncode != 0:
        raise RuntimeError(f"Run failed (N={N}) rc={p.returncode}\nSTDERR:\n{p.stderr}\nSTDOUT:\n{out}")

    m_seq = NS_RE.search(out)
    m_pipe = NP_RE.search(out)
    m_sp = SP_RE.search(out)

    if not m_seq or not m_pipe:
        raise RuntimeError(f"Could not parse times from output (N={N}). Output:\n{out}")

    seq_str = m_seq.group(1).strip()
    pipe_str = m_pipe.group(1).strip()

    seq_ms = parse_human_time_to_ms(seq_str)
    pipe_ms = parse_human_time_to_ms(pipe_str)

    speedup = float(m_sp.group(1)) if m_sp else (seq_ms / pipe_ms if pipe_ms > 0 else float("inf"))

    return {
        "N": N,
        "sequential_str": seq_str,
        "pipeline_str": pipe_str,
        "sequential_ms": seq_ms,
        "pipeline_ms": pipe_ms,
        "speedup": speedup,
        "raw_stdout": out
    }

def main():
    exe = "./app.exe"
    filelist = "list.txt"
    M = 2
    eps = 1.0
    minPts = 2
    k = 4 + 3
    directed = 1
    prefix = "out_"

    Ns = [25, 50, 75, 100, 125]

    os.makedirs("ready", exist_ok=True)

    results = []
    for N in Ns:
        print(f"[RUN] N={N} ...")
        r = run_once(exe, filelist, N, M, eps, minPts, k, directed, prefix)
        results.append(r)
        print(f"      seq={r['sequential_str']}  pipe={r['pipeline_str']}  speedup={r['speedup']:.3f}")

        with open(f"ready/sample_log_N{N}.txt", "w", encoding="utf-8") as f:
            f.write(r["raw_stdout"])

    csv_path = "ready/pipeline_experiment.csv"
    with open(csv_path, "w", newline="", encoding="utf-8") as f:
        w = csv.writer(f)
        w.writerow(["timestamp", "N", "sequential_str", "pipeline_str", "sequential_ms", "pipeline_ms", "speedup"])
        ts = datetime.now().isoformat(timespec="seconds")
        for r in results:
            w.writerow([ts, r["N"], r["sequential_str"], r["pipeline_str"], f"{r['sequential_ms']:.6f}", f"{r['pipeline_ms']:.6f}", f"{r['speedup']:.6f}"])

    json_path = "ready/pipeline_experiment.json"
    payload = {
        "timestamp": datetime.now().isoformat(timespec="seconds"),
        "exe": exe,
        "filelist": filelist,
        "params": {"M": M, "eps": eps, "minPts": minPts, "k": k, "directed": directed, "prefix": prefix},
        "results": [
            {k: v for k, v in r.items() if k != "raw_stdout"} for r in results
        ],
        "sample_logs": [f"ready/sample_log_N{r['N']}.txt" for r in results]
    }
    with open(json_path, "w", encoding="utf-8") as f:
        json.dump(payload, f, ensure_ascii=False, indent=2)

    print("\n[OK]")
    print(f"CSV:  {csv_path}")
    print(f"JSON: {json_path}")
    print("Sample logs saved as ready/sample_log_N*.txt")


if __name__ == "__main__":
    main()
