import subprocess


def build(target, sources):
    subprocess.run(["g++", "-pthread", *sources, "-o", f"ready/{target}"], check=True)


build("app-tui-release", ["code/main.cpp", "code/graph_dbscan.cpp", "code/cpu_time.cpp"])
