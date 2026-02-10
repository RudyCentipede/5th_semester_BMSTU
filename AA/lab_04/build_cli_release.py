import subprocess


def build(target, sources):
    subprocess.run(["g++", "-pthread", *sources, "-o", f"ready/{target}"], check=True)


build("app-cli-release", ["code/app-cli.cpp", "code/graph_dbscan.cpp"])
