#!/usr/bin/env python3
"""Create a standalone HTML summary from campaign JSON reports."""
import argparse
import html
import json
from pathlib import Path


def render(paths):
    rows = []
    for path in paths:
        report = json.loads(path.read_text(encoding="utf-8"))
        observations = report["observations"]
        detail = "\n".join(f"{o['compiler']} {o['optimization']}: exit {o['execution']['exit_code']}\n"
                           f"stdout: {o['execution']['stdout']}\nstderr: {o['execution']['stderr']}"
                           for o in observations)
        rows.append("<tr><td>" + html.escape(str(path)) + "</td><td>" + str(report["seed"]) +
                    "</td><td>" + html.escape(report["status"]) + "</td><td><pre>" +
                    html.escape(detail) + "</pre></td></tr>")
    return ("<!doctype html><html lang='en'><meta charset='utf-8'><title>A64Sentinel reports</title>"
            "<style>body{font:16px system-ui;margin:2em}table{border-collapse:collapse;width:100%}"
            "td,th{border:1px solid #ccc;padding:.6em;text-align:left}pre{white-space:pre-wrap}</style>"
            "<h1>A64Sentinel regression reports</h1><table><tr><th>Report</th><th>Seed</th>"
            "<th>Status</th><th>Observed behavior</th></tr>" + "".join(rows) + "</table></html>")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("campaign", type=Path)
    parser.add_argument("--out", required=True, type=Path)
    args = parser.parse_args()
    paths = sorted(args.campaign.rglob("report.json"))
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(render(paths), encoding="utf-8")
    print(f"wrote {args.out} ({len(paths)} reports)")


if __name__ == "__main__":
    main()
