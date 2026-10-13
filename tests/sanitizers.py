"""Audit generated programs under ASan/UBSan; intended for Linux CI."""
import subprocess
import sys
import tempfile
from pathlib import Path


def main():
    executable = Path(sys.argv[1]).resolve()
    compilers = sys.argv[2:] or ["gcc", "clang"]
    with tempfile.TemporaryDirectory(prefix="sentinel-sanitizers-") as temp:
        root = Path(temp)
        for cxx in (False, True):
            directory = root / ("cxx" if cxx else "c")
            cmd = [str(executable), "--mode", "generate", "--count", "12", "--out", str(directory)]
            if cxx:
                cmd.append("--cxx")
            subprocess.run(cmd, check=True)
            for source in directory.glob("seed-*/source.*"):
                for compiler in compilers:
                    binary = root / "audit"
                    subprocess.run([compiler, "-x", "c++" if cxx else "c",
                                    "-std=c++17" if cxx else "-std=c11", "-O2",
                                    "-fsanitize=address,undefined", "-fno-sanitize-recover=all",
                                    str(source), "-o", str(binary)], check=True, capture_output=True)
                    result = subprocess.run([str(binary)], check=True, capture_output=True, timeout=10)
                    assert not result.stderr, (source, compiler, result.stderr)
    print("ASan/UBSan audit passed for 24 generated programs per compiler")


if __name__ == "__main__":
    main()
