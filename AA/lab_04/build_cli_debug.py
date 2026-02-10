import subprocess


def build(target, sources):
    subprocess.run(["g++", "-O0", "-g", "-pthread", *sources, "-o", f"ready/{target}"], check=True)


build("app-cli-debug", ["code/app-cli.cpp", "code/graph_dbscan.cpp"])
