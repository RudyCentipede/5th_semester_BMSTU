import subprocess


def build(target, sources):
    subprocess.run(["g++", "-O0", "-g",  *sources, "-o", f"ready/{target}"], check=True)


build("app-cli-debug", ["code/param_main.cpp", "code/salesman.cpp"])
