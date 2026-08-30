#!/usr/bin/env python3
"""Traces digit shapes from a nixie reference image into stroke skeletons.

Extracts each digit's bright core, skeletonizes it to its medial axis,
decomposes the skeleton into ordered stroke paths, and writes them as
normalized point lists (JSON) that build_font.py renders with its own
uniform round stroke. The image is only a geometric guide; the committed
traced_skeletons.json is what the build consumes.

Usage:  python3 trace_ref.py <reference.jpg> <out.json>
        (expects a 5x2 grid of digits 0-4 / 5-9)
"""
import json
import math
import sys

import numpy as np
from PIL import Image
from skimage.morphology import skeletonize, binary_closing, disk, remove_small_objects

ADVANCE = 620
CAP = 700
BASE_R = 48  # stroke radius used by build_font, kept as margin


def digit_mask(cell):
    """Bright-core mask of one digit cell (float grayscale 0..1)."""
    g = cell
    t = g.max() * 0.62
    m = g > t
    m = binary_closing(m, disk(5))  # bridge the dark cage lines
    m = remove_small_objects(m, 80)
    return m


def skeleton_paths(mask):
    """Ordered point paths of the mask's medial axis."""
    sk = skeletonize(mask)
    pts = set(map(tuple, np.argwhere(sk)))

    def neighbors(p):
        (r, c) = p
        out = []
        for dr in (-1, 0, 1):
            for dc in (-1, 0, 1):
                if dr == 0 and dc == 0:
                    continue
                q = (r + dr, c + dc)
                if q in pts:
                    out.append(q)
        return out

    degree = {p: len(neighbors(p)) for p in pts}
    junctions = {p for p, d in degree.items() if d >= 3}
    core = pts - junctions

    # connected components of the junction-free skeleton = edges
    unvisited = set(core)
    paths = []
    while unvisited:
        seed = next(iter(unvisited))
        comp = {seed}
        stack = [seed]
        while stack:
            p = stack.pop()
            for q in neighbors(p):
                if q in unvisited and q not in comp:
                    comp.add(q)
                    stack.append(q)
        unvisited -= comp

        # order the component by walking from an endpoint (or anywhere, if loop)
        ends = [p for p in comp if sum(1 for q in neighbors(p) if q in comp) <= 1]
        start = ends[0] if ends else next(iter(comp))
        path = [start]
        seen = {start}
        cur = start
        while True:
            nxt = [q for q in neighbors(cur) if q in comp and q not in seen]
            if not nxt:
                break
            cur = min(nxt, key=lambda q: (q[0] - path[-1][0]) ** 2 + (q[1] - path[-1][1]) ** 2)
            path.append(cur)
            seen.add(cur)
        # reattach adjacent junction points so edges meet
        j0 = [j for j in junctions if j in set(neighbors(path[0]))]
        j1 = [j for j in junctions if j in set(neighbors(path[-1]))]
        if j0:
            path.insert(0, j0[0])
        if j1:
            path.append(j1[0])
        if len(path) >= 6:  # drop tiny spurs
            paths.append(path)
    return paths


def smooth(path, w=11):
    out = []
    for i in range(len(path)):
        lo, hi = max(0, i - w // 2), min(len(path), i + w // 2 + 1)
        rs = sum(p[0] for p in path[lo:hi]) / (hi - lo)
        cs = sum(p[1] for p in path[lo:hi]) / (hi - lo)
        out.append((rs, cs))
    return out


def trace(image_path):
    img = Image.open(image_path).convert("L")
    a = np.asarray(img, dtype=float) / 255.0
    h, w = a.shape
    glyphs = {}
    names = ["zero", "one", "two", "three", "four",
             "five", "six", "seven", "eight", "nine"]
    for row in range(2):
        for col in range(5):
            name = names[row * 5 + col]
            y0, y1 = int(h * (0.04 + row * 0.48)), int(h * (0.46 + row * 0.48))
            x0, x1 = int(w * (col * 0.192 + 0.03)), int(w * (col * 0.192 + 0.20))
            cell = a[y0:y1, x0:x1]
            mask = digit_mask(cell)
            paths = [smooth(smooth(p)) for p in skeleton_paths(mask)
                     if len(p) >= 10]
            if not paths:
                raise SystemExit("no strokes traced for " + name)

            # normalize: uniform scale to cap height, centered in the advance
            allp = [p for path in paths for p in path]
            rmin = min(p[0] for p in allp)
            rmax = max(p[0] for p in allp)
            cmin = min(p[1] for p in allp)
            cmax = max(p[1] for p in allp)
            s = (CAP - 2 * BASE_R) / max(1.0, rmax - rmin)
            xoff = ADVANCE / 2 - s * (cmax + cmin) / 2
            strokes = []
            for path in paths:
                pick = path[::3]
                if path[-1] not in pick:
                    pick.append(path[-1])
                stroke = [(round(xoff + s * c, 1),
                           round(BASE_R + s * (rmax - r), 1))
                          for (r, c) in pick]
                strokes.append(stroke)
            glyphs[name] = strokes
    return glyphs


def main():
    glyphs = trace(sys.argv[1])
    with open(sys.argv[2], "w") as f:
        json.dump(glyphs, f)
    for name, strokes in glyphs.items():
        print(name, len(strokes), "strokes,",
              sum(len(s) for s in strokes), "points")


if __name__ == "__main__":
    main()
