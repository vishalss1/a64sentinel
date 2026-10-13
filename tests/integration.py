import importlib.util
import json
import subprocess
import sys
import tempfile
from pathlib import Path


def run(args, expected=0):
    r = subprocess.run(list(map(str, args)), capture_output=True, text=True)
    assert r.returncode == expected, (args, r.returncode, r.stdout, r.stderr)
    return r


def main():
    exe, fixture, gcc, clang = sys.argv[1:]
    with tempfile.TemporaryDirectory(prefix="sentinel integration spaces ") as temp:
        root = Path(temp)
        for cxx in (False, True):
            out = root / ("cxx" if cxx else "c")
            args = [exe, "--count", "3", "--compiler", gcc, "--compiler", clang,
                    "--out", out, "--no-minimize"] + (["--cxx"] if cxx else [])
            run(args)
            for path in out.rglob("report.json"):
                report = json.loads(path.read_text())
                assert report["status"] == "pass"
                assert len(report["observations"]) == 6
        out = root / "failure"
        run([exe, "--compiler", fixture, "--opt", "-O0", "--opt", "-O2", "--out", out], 1)
        case = out / "seed-1"
        report = json.loads((case / "minimized-report.json").read_text())
        assert report["status"] == "mismatch"
        minimized = (case / "minimized.c").read_text()
        assert minimized.count("/* BEGIN block ") == 1
        assert "/* BEGIN block 5 */" in minimized
        for cfg in range(2):
            work = case / "minimized" / f"config-{cfg}"
            assert (work / "program.s").stat().st_size > 0
            assert (work / "program.ll").stat().st_size > 0
        scripts = Path(__file__).resolve().parents[1] / "python"
        run([sys.executable, scripts / "regressions.py", case / "minimized-report.json",
             "--executable", exe, "--out", root / "replay", "--expect", "mismatch"])
        run([sys.executable, scripts / "report.py", out, "--out", root / "report.html"])
        assert "mismatch" in (root / "report.html").read_text()
        run([exe, "--out", out], 2)  # Never overwrite an existing case.
        run([exe, "--count", "-1"], 2)
        run([exe, "--opt", "-O9"], 2)
        run([exe, "--compiler", "missing-sentinel-compiler", "--out", root / "missing"], 2)
        missing = json.loads((root / "missing" / "seed-1" / "report.json").read_text())
        assert missing["status"] != "mismatch"
    print("native C/C++, synthetic reduction, diagnostics, replay and validation passed")


if __name__ == "__main__":
    main()
