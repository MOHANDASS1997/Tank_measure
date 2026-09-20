#!/usr/bin/env python3
"""
Generate WebPortalHtml.h from web_portal.html with gzip compression.
Run this script whenever you edit receiver/wifi/web_portal.html.
"""

import gzip
import os

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
HTML_PATH = os.path.join(SCRIPT_DIR, "web_portal.html")
HEADER_PATH = os.path.join(SCRIPT_DIR, "WebPortalHtml.h")

def main():
    if not os.path.exists(HTML_PATH):
        print(f"Error: {HTML_PATH} not found.")
        return 1

    with open(HTML_PATH, "r", encoding="utf-8") as f:
        html = f.read().strip()

    gz = gzip.compress(html.encode("utf-8"), compresslevel=9)
    orig_len = len(html.encode("utf-8"))
    gz_len = len(gz)
    savings = (1.0 - (gz_len / orig_len)) * 100.0

    print(f"Original HTML:   {orig_len:,} bytes")
    print(f"Gzipped Payload: {gz_len:,} bytes")
    print(f"Flash Savings:   {savings:.1f}%")

    header_lines = [
        "#pragma once",
        "",
        "#include <Arduino.h>",
        "",
        "// =====================================================",
        "//      DASS HOME WEB PORTAL (GZIPPED PROGMEM ASSET)",
        "// =====================================================",
        f"// Source file: receiver/wifi/web_portal.html",
        f"// Regenerate using: python3 receiver/wifi/generate_portal_gz.py",
        f"// Original size: {orig_len} bytes",
        f"// Compressed size: {gz_len} bytes (~{savings:.1f}% savings)",
        "",
        "const uint8_t WEB_PORTAL_HTML_GZ[] PROGMEM = {"
    ]

    hex_bytes = [f"0x{b:02x}" for b in gz]
    for i in range(0, len(hex_bytes), 16):
        line = "  " + ", ".join(hex_bytes[i:i + 16])
        if i + 16 < len(hex_bytes):
            line += ","
        header_lines.append(line)

    header_lines.append("};")
    header_lines.append("")
    header_lines.append("const size_t WEB_PORTAL_HTML_GZ_LEN = sizeof(WEB_PORTAL_HTML_GZ);")
    header_lines.append("")

    with open(HEADER_PATH, "w", encoding="utf-8") as f:
        f.write("\n".join(header_lines))

    print(f"Wrote {HEADER_PATH}")
    return 0

if __name__ == "__main__":
    exit(main())
