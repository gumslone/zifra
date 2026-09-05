#!/usr/bin/env python3
"""Regenerates zifra/src/Webinterface.h: everything the clock serves itself.

  zifra/pages/index.html  -> MAIN_PAGE_GZ   (the app shell, {{version}} filled in)
  zifra/pages/update.html -> UPDATE_PAGE_GZ (fallback flasher page)
  web/v2 JS bundle        -> APP_JS_GZ      (same file list as js/javascript.php)
  web/v2 CSS bundle       -> APP_CSS_GZ     (same file list as css/style.php)

Each blob is gzipped deterministically (level 9, zeroed mtime) into a
PROGMEM byte array served with Content-Encoding: gzip. build.sh runs this
before every firmware build; run it by hand after editing a page or the
web app:  python3 tools/web/gzip_pages.py
"""
import gzip
import os
import re

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
WEB = os.path.join(ROOT, "web", "v2")


def read(path):
    with open(path, "rb") as f:
        return f.read()


def version():
    src = read(os.path.join(ROOT, "zifra", "src", "version.h")).decode()
    return re.search(r'ZIFRA_VERSION\s+"([^"]+)"', src).group(1)


def php_bundle(bundler):
    """Concatenates the files a PHP bundler lists, in its order."""
    src = read(bundler).decode()
    files = re.findall(r"'\./files/([^']+)'", src)
    base = os.path.dirname(bundler)
    return b"\n".join(read(os.path.join(base, "files", f)) for f in files) + b"\n"


# ---- minifiers ---------------------------------------------------------
# Comments and indentation only serve the sources in web/v2; the embedded
# copy drops them before gzip. Deliberately conservative: line breaks stay
# (JS relies on them for semicolon insertion) and strings are copied
# verbatim, so a "//" inside a URL or an HTML template survives.

_REGEX_BEFORE = set("(,=:[!&|?{};\n")


def strip_js(src):
    out = []
    i = 0
    n = len(src)
    last = "\n"  # last significant character, decides regex vs division
    while i < n:
        c = src[i]
        if c in "'\"`":
            j = i + 1
            while j < n:
                if src[j] == "\\":
                    j += 2
                    continue
                if src[j] == c:
                    break
                j += 1
            out.append(src[i:j + 1])
            last = c
            i = j + 1
            continue
        if c == "/" and src.startswith("//", i):
            j = src.find("\n", i)
            i = n if j < 0 else j
            continue
        if c == "/" and src.startswith("/*", i):
            j = src.find("*/", i + 2)
            i = n if j < 0 else j + 2
            continue
        if c == "/" and last in _REGEX_BEFORE:
            j = i + 1
            in_class = False
            while j < n:
                if src[j] == "\\":
                    j += 2
                    continue
                if src[j] == "[":
                    in_class = True
                elif src[j] == "]":
                    in_class = False
                elif src[j] == "/" and not in_class:
                    break
                j += 1
            out.append(src[i:j + 1])
            last = "/"
            i = j + 1
            continue
        out.append(c)
        if not c.isspace():
            last = c
        i += 1
    lines = [line.rstrip() for line in "".join(out).split("\n")]
    lines = [line.lstrip() for line in lines]
    return "\n".join(line for line in lines if line)


def strip_css(src):
    src = re.sub(r"/\*.*?\*/", "", src, flags=re.S)
    lines = [line.strip() for line in src.split("\n")]
    return "\n".join(line for line in lines if line)


def strip_html(src):
    src = re.sub(r"<!--.*?-->", "", src, flags=re.S)
    lines = [line.strip() for line in src.split("\n")]
    return "\n".join(line for line in lines if line)


def minify(kind, raw):
    text = raw.decode("utf-8")
    text = {"js": strip_js, "css": strip_css, "html": strip_html}[kind](text)
    return (text + "\n").encode("utf-8")


def to_array(name, raw, origin):
    gz = gzip.compress(raw, compresslevel=9, mtime=0)
    lines = []
    for i in range(0, len(gz), 12):
        lines.append("  " + ", ".join("0x%02x" % b for b in gz[i:i + 12]) + ",")
    return (
        "\n// %s\n// %d bytes gzipped from %d\n"
        "const uint8_t %s_GZ[] PROGMEM = {\n%s\n};\n"
        "constexpr size_t %s_GZ_LEN = sizeof(%s_GZ);\n"
        % (origin, len(gz), len(raw), name, "\n".join(lines), name, name)
    ), len(raw), len(gz)


def main():
    ver = version()
    index = read(os.path.join(ROOT, "zifra", "pages", "index.html"))
    index = index.replace(b"{{version}}", ver.encode())
    blobs = [
        ("MAIN_PAGE", minify("html", index), "zifra/pages/index.html (v" + ver + ")"),
        ("UPDATE_PAGE", minify("html", read(os.path.join(ROOT, "zifra", "pages", "update.html"))),
         "zifra/pages/update.html"),
        ("APP_JS", minify("js", php_bundle(os.path.join(WEB, "js", "javascript.php"))),
         "web/v2/js bundle"),
        ("APP_CSS", minify("css", php_bundle(os.path.join(WEB, "css", "style.php"))),
         "web/v2/css bundle"),
    ]
    if os.environ.get("ZIFRA_DUMP_BUNDLES"):  # for checking the minified output
        for name, raw, _ in blobs:
            with open(os.path.join(os.environ["ZIFRA_DUMP_BUNDLES"], name.lower() + ".out"), "wb") as f:
                f.write(raw)
    parts = [
        "#pragma once\n"
        "#ifndef ZIFRA_WEBINTERFACE_H\n"
        "#define ZIFRA_WEBINTERFACE_H\n\n"
        "// GENERATED by tools/web/gzip_pages.py from zifra/pages/ and web/v2 -\n"
        "// edit those and rerun the script; do not edit this file.\n"
        "// Served with Content-Encoding: gzip (webservice.h).\n"
    ]
    total = 0
    for name, raw, origin in blobs:
        block, n_raw, n_gz = to_array(name, raw, origin)
        parts.append(block)
        total += n_gz
        print("%-24s %6d -> %5d bytes" % (origin.split(" (")[0], n_raw, n_gz))
    parts.append("\n#endif\n")
    out = os.path.join(ROOT, "zifra", "src", "Webinterface.h")
    with open(out, "w") as f:
        f.write("".join(parts))
    print("total %d bytes of PROGMEM, wrote %s" % (total, os.path.relpath(out, ROOT)))


if __name__ == "__main__":
    main()
