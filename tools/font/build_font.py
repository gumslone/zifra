#!/usr/bin/env python3
"""Builds the "Zifra Tube" webfont: nixie-style digits for the v2 web UI.

Each glyph is a skeleton of line segments and circular arcs, like the bent
wire cathodes of an IN-12B tube. The skeleton is sampled and emitted as
overlapping thick segments plus joint circles; TrueType's nonzero winding
unions them into one smooth round-capped stroke.

Glyph set: 0-9, colon, hyphen, period, space - everything the clock UI shows.

Usage:  python3 build_font.py <outdir>   (writes zifra-tube.ttf and .woff)
"""
import math
import sys

from fontTools.fontBuilder import FontBuilder
from fontTools.pens.ttGlyphPen import TTGlyphPen

UPM = 1000        # units per em
STROKE = 96       # stroke thickness
R = STROKE / 2
ADVANCE = 620     # default advance width
SEG = 16          # skeleton sampling step, font units


def line(p1, p2):
    return ("line", p1, p2)


def arc(c, r, a0, a1):
    """Circular arc around c, from angle a0 to a1 in degrees (a0 > a1 means
    clockwise). Angles: 0 = right, 90 = up."""
    return ("arc", c, r, a0, a1)


# ---- glyph skeletons ------------------------------------------------------
# Coordinates: baseline y=0, cap height ~700, x roughly 80..540.

SKELETONS = {
    "zero": [[arc((310, 350), 1.0, 90, 450)]],  # ellipse ring, see sample()
    # real nixie tubes show 1 as a plain vertical wire, no flag
    "one": [[line((310, 700), (310, 0))]],
    "two": [[arc((310, 495), 190, 165, 15),
             line((494, 544), (95, 0)),
             line((95, 0), (530, 0))]],
    # Soviet tubes use the flat-top 3: bar, diagonal to the waist, bowl
    "three": [[line((150, 700), (465, 700)),
               line((465, 700), (330, 430)),
               arc((295, 235), 198, 80, -160)]],
    "four": [[line((400, 700), (120, 240)), line((120, 240), (530, 240))],
             [line((400, 700), (400, 0))]],
    "five": [[line((490, 700), (155, 700)),
              line((155, 700), (155, 435)),
              arc((310, 240), 195, 95, -160)]],
    "six": [[arc((350, 460), 245, 85, 190),
             line((109, 417), (139, 300)),
             arc((300, 195), 175, 143, -217)]],
    # the stem bows gently left, like the reference tubes
    "seven": [[line((100, 700), (520, 700)),
               arc((1994, -308), 1786, 145.6, 170.1)]],
    "eight": [[arc((310, 510), 1.02, 90, 450)],
              [arc((310, 172), 1.08, 90, 450)]],
    "nine": [[arc((320, 505), 178, 90, 450)],
             [line((488, 442), (466, 447)),
              arc((150, 320), 340, 22, -42)]],
    "colon": [[line((260, 460), (260, 460))], [line((260, 120), (260, 120))]],
    "hyphen": [[line((140, 300), (400, 300))]],
    "period": [[line((180, 50), (180, 50))]],
}

# per-glyph advance overrides; digits all share ADVANCE so times are
# tabular, like the fixed anode width of a real tube
ADVANCES = {"colon": 520, "period": 360, "space": 420}

# rings drawn as ellipses: (rx, ry) selected by the arc's r acting as a marker
ELLIPSES = {
    "zero": (200, 310),
    "eight-top": (155, 165),
    "eight-bottom": (180, 172),
}


def sample(glyph_name, stroke_index, part):
    """Returns a list of points sampling one skeleton part."""
    kind = part[0]
    if kind == "line":
        (x1, y1), (x2, y2) = part[1], part[2]
        length = math.hypot(x2 - x1, y2 - y1)
        n = max(1, int(length / SEG))
        return [(x1 + (x2 - x1) * i / n, y1 + (y2 - y1) * i / n)
                for i in range(n + 1)]
    c, r, a0, a1 = part[1], part[2], part[3], part[4]
    if r <= 2.0:  # marker: draw an ellipse ring instead of a circular arc
        key = {1.0: "zero", 1.02: "eight-top", 1.08: "eight-bottom"}[r]
        rx, ry = ELLIPSES[key]
        perimeter = math.pi * (3 * (rx + ry) - math.sqrt((3 * rx + ry) * (rx + 3 * ry)))
        n = max(64, int(perimeter / SEG) * 2)
        return [(c[0] + rx * math.cos(math.radians(a)),
                 c[1] + ry * math.sin(math.radians(a)))
                for a in (a0 + (a1 - a0) * i / n for i in range(n + 1))]
    span = abs(a1 - a0)
    n = max(2, int(math.radians(span) * r / SEG))
    return [(c[0] + r * math.cos(math.radians(a)),
             c[1] + r * math.sin(math.radians(a)))
            for a in (a0 + (a1 - a0) * i / n for i in range(n + 1))]


def draw_circle(pen, c, r):
    """Circle as a TrueType quadratic contour, wound CLOCKWISE to match
    draw_segment's quads - mixed windings cancel where shapes overlap."""
    x, y = c
    pen.moveTo((x + r, y))
    for i in range(8):
        a_mid = math.radians(-(45 * i + 22.5))
        a_end = math.radians(-45 * (i + 1))
        pen.qCurveTo(
            (x + r / math.cos(math.pi / 8) * math.cos(a_mid),
             y + r / math.cos(math.pi / 8) * math.sin(a_mid)),
            (x + r * math.cos(a_end), y + r * math.sin(a_end)))
    pen.closePath()


def draw_segment(pen, p1, p2):
    """Thick line segment as a rectangle contour."""
    dx, dy = p2[0] - p1[0], p2[1] - p1[1]
    length = math.hypot(dx, dy)
    if length < 0.01:
        return
    nx, ny = -dy / length * R, dx / length * R
    pen.moveTo((p1[0] + nx, p1[1] + ny))
    pen.lineTo((p2[0] + nx, p2[1] + ny))
    pen.lineTo((p2[0] - nx, p2[1] - ny))
    pen.lineTo((p1[0] - nx, p1[1] - ny))
    pen.closePath()


def build_glyph(name):
    pen = TTGlyphPen(None)
    for si, stroke in enumerate(SKELETONS[name]):
        pts = []
        for part in stroke:
            seg_pts = sample(name, si, part)
            if pts and seg_pts and pts[-1] == seg_pts[0]:
                seg_pts = seg_pts[1:]
            pts.extend(seg_pts)
        dot_r = R * 1.35 if len(pts) == 1 else R
        for p in pts:
            draw_circle(pen, p, dot_r)
        for a, b in zip(pts, pts[1:]):
            draw_segment(pen, a, b)
    return pen.glyph()


def main(outdir):
    order = ["zero", "one", "two", "three", "four", "five", "six", "seven",
             "eight", "nine", "colon", "hyphen", "period"]
    cmap = {ord("0") + i: order[i] for i in range(10)}
    cmap[ord(":")] = "colon"
    cmap[ord("-")] = "hyphen"
    cmap[0x2013] = "hyphen"  # en dash, used in the sleep-window display
    cmap[0x2014] = "hyphen"
    cmap[ord(".")] = "period"
    cmap[ord(" ")] = "space"

    glyph_order = [".notdef", "space"] + order
    glyphs = {".notdef": TTGlyphPen(None).glyph(),
              "space": TTGlyphPen(None).glyph()}
    for name in order:
        glyphs[name] = build_glyph(name)

    metrics = {}
    for name in glyph_order:
        metrics[name] = (ADVANCES.get(name, ADVANCE), 60)
    metrics[".notdef"] = (ADVANCE, 0)
    metrics["space"] = (ADVANCES["space"], 0)

    fb = FontBuilder(UPM, isTTF=True)
    fb.setupGlyphOrder(glyph_order)
    fb.setupCharacterMap(cmap)
    fb.setupGlyf(glyphs)
    fb.setupHorizontalMetrics(metrics)
    fb.setupHorizontalHeader(ascent=800, descent=-200)
    fb.setupNameTable({
        "familyName": "Zifra Tube",
        "styleName": "Regular",
        "fullName": "Zifra Tube Regular",
        "psName": "ZifraTube-Regular",
        "version": "1.0",
        "copyright": "gumslone, generated by tools/font/build_font.py (MIT)",
    })
    fb.setupOS2(sTypoAscender=800, sTypoDescender=-200, usWinAscent=800,
                usWinDescent=200)
    fb.setupPost()

    fb.save(outdir + "/zifra-tube.ttf")
    fb.font.flavor = "woff"
    fb.save(outdir + "/zifra-tube.woff")
    print("wrote", outdir + "/zifra-tube.ttf", "and .woff")


if __name__ == "__main__":
    main(sys.argv[1] if len(sys.argv) > 1 else ".")
