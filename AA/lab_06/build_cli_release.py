import subprocess


def build(target, sources):
    subprocess.run(["g++", *sources, "-o", f"ready/{target}"], check=True)


build("app-cli-release", ["code/param_main.cpp", "code/salesman.cpp"])
