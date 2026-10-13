#!/usr/bin/env python3
"""Replay reports using their exact compiler configurations, without a shell."""
import argparse
import json
import subprocess
import sys
from pathlib import Path


def replay(report_path, executable, output):
    report = json.loads(report_path.read_text(encoding="utf-8"))
    if report.get("schema_version") != 1:
        raise ValueError("unsupported report schema")
    source = (report_path.parent / report["source"]).resolve()
    results = []
    # Duplicate each configuration to satisfy the native differential contract;
    # compare the original configurations after validating both executions.
    for index, cfg in enumerate(report["observations"]):
        cmd = [str(executable), "--source", str(source), "--seed", str(report["seed"]),
               "--out", str(output / str(index)), "--compiler", cfg["compiler"],
               "--opt", cfg["optimization"], "--opt", cfg["optimization"],
               "--target", cfg["target"], "--timeout-ms", str(report["timeout_ms"]), "--no-minimize"]
        if cfg["cxx"]:
            cmd.append("--cxx")
        if cfg["cpu"]:
            cmd += ["--cpu", cfg["cpu"]]
        for flag in cfg["flags"]:
            cmd += ["--flag", flag]
        if cfg["runner"]:
            cmd += ["--runner", cfg["runner"][0]]
            for arg in cfg["runner"][1:]:
                cmd += ["--runner-arg", arg]
        completed = subprocess.run(cmd, check=False)
        path = output / str(index) / f"seed-{report['seed']}" / "report.json"
        if not path.exists():
            raise RuntimeError(f"replay produced no report (exit {completed.returncode})")
        observed = json.loads(path.read_text())
        if observed["status"] not in ("pass", "runtime_error"):
            return "inconclusive"
        results.append(observed["observations"][0]["execution"])
    if len(results) < 2:
        return "inconclusive"
    signatures = {(r["exit_code"], r["stdout"], r["stderr"]) for r in results}
    if len(signatures) > 1:
        return "mismatch"
    return "pass" if results[0]["exit_code"] == 0 else "runtime_error"


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("reports", nargs="+", type=Path)
    parser.add_argument("--executable", required=True, type=Path)
    parser.add_argument("--out", required=True, type=Path)
    parser.add_argument("--expect", choices=["pass", "mismatch"], default="pass")
    args = parser.parse_args()
    failed = False
    for index, path in enumerate(args.reports):
        status = replay(path.resolve(), args.executable.resolve(), args.out.resolve() / str(index))
        print(f"{path}: {status}")
        failed |= status != args.expect
    return int(failed)


if __name__ == "__main__":
    try:
        sys.exit(main())
    except (OSError, ValueError, KeyError, RuntimeError) as exc:
        print(f"regressions: {exc}", file=sys.stderr)
        sys.exit(2)
