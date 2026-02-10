import random
import subprocess
from pathlib import Path


def generate_graph(n, max_degree, directed, filename):
    edges = set()
    for u in range(n):
        degree = random.randint(0, max_degree)
        for _ in range(degree):
            v = random.randint(0, n - 1)
            if v == u:
                continue
            if directed:
                edges.add((u, v))
            else:
                e = tuple(sorted((u, v)))
                edges.add(e)

    with open(filename, "w") as f:
        f.write("digraph G {\n" if directed else "graph G {\n")
        connector = "->" if directed else "--"
        for (u, v) in edges:
            f.write(f"  v{u} {connector} v{v};\n")

        for u in range(n):
            f.write(f"  v{u};\n")
        f.write("}\n")

    print(f"Graph created: {filename} ({len(edges)} edges, {n} nodes).")
    return filename


def to_svg(dot_file):
    svg_file = Path(dot_file).with_suffix(".svg")
    try:
        subprocess.run(["dot", "-Tsvg", dot_file, "-o", str(svg_file)],
                       check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        print(f"SVG created: {svg_file}")
    except subprocess.CalledProcessError as e:
        print(f"Failed to create SVG: {e}")
        print(e.stderr.decode("utf-8", errors="ignore"))


if __name__ == "__main__":
    n = 50
    max_degree = 4
    directed = True

    filename = "test_graph.dot"
    dot_file = generate_graph(n, max_degree, directed, filename)
    to_svg(dot_file)
