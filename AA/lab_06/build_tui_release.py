import subprocess


def build(target, sources):
    subprocess.run(["g++", *sources, "-o", f"ready/{target}"], check=True)


build("app-tui-release", ["code/main.cpp", "code/salesman.cpp"])
