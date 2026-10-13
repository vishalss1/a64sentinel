# A64Sentinel

A C++17 differential compiler tester for C11 and C++17 programs, with AArch64 targeting, bounded failure reduction, and reproducible diagnostics. It compares stdout, stderr, and exit codes across GCC/Clang optimization configurations. A mismatch is a **potential** compiler defect requiring investigation.

## Build and test

Requires CMake 3.20+, a C++17 compiler, GCC and Clang for differential testing, and Python 3.9+ for report/replay tools. GoogleTest v1.15.2 is fetched when testing is enabled unless an installed package is found.

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

On Windows with MSYS2 UCRT64, select `-DCMAKE_CXX_COMPILER=clang++` or `g++`; the toolchain directory must be on PATH. MinGW framework executables link their runtime statically. MSVC can build the framework; generated programs still need GCC/Clang drivers. Clang MinGW builds also require lld.

Offline: install GoogleTest and use `-DSENTINEL_FETCH_GTEST=OFF`. For a tool-only build, use `-DBUILD_TESTING=OFF`. Integration tests are registered when both GCC and Clang are found.

## Native campaigns

```sh
build/a64sentinel --seed 1 --count 100 --out out/native
build/a64sentinel --seed 1000 --count 20 --cxx --out out/cxx
build/a64sentinel --compiler gcc-14 --compiler clang-18 --opt -O0 --opt -O3 --out out/versions
build/a64sentinel --mode generate --seed 42 --out out/generated
```

Use `build/a64sentinel.exe` on Windows. Each supplied compiler is crossed with the requested optimization levels (default `-O0`, `-O2`, `-O3`). `--flag ARG` adds an individual compiler argument and can be repeated; arguments are passed directly without a shell. Compiler executable paths with spaces are supported. Campaigns are sequential and every case gets an isolated directory. Existing case directories are rejected.

Exit codes: `0` all cases pass/generated, `1` at least one runtime mismatch, `2` configuration, compile, runtime, or inconclusive errors without a mismatch. Timeouts, spawn failures, and truncated output cannot be classified as miscompilations.

## AArch64 and Grace

On an AArch64 Linux machine, including a Grace system:

```sh
build/a64sentinel --target aarch64 --cpu neoverse-v2 --count 100 --out out/grace
```

For Linux cross execution, install AArch64 GCC, Clang, the target C runtime/sysroot, and QEMU user mode:

```sh
build/a64sentinel --target aarch64 --compiler aarch64-linux-gnu-gcc --compiler clang   --cpu neoverse-v2 --flag -static --runner qemu-aarch64 --count 10 --out out/aarch64
```

Clang receives `--target=aarch64-linux-gnu`. Non-Clang compilers must report an AArch64/ARM64 target using `-dumpmachine`; a host GCC is rejected. Clang must discover the cross GCC libraries or receive a suitable shared `--flag` setting. Without static linkage, pass QEMU `--runner-arg -L --runner-arg /usr/aarch64-linux-gnu`. Runner arguments such as `--runner-arg -cpu --runner-arg max` permit vector instruction emulation. Use flags supported by every compiler in that campaign; separate campaigns/replay recipes can carry different compiler flags.

The generator exercises alignment, legal byte access, offset loads, alias dependencies, branches, and vectorizable integer loops. `--cpu` allows architecture instruction-selection experiments. QEMU validates emulated semantics; actual Grace CPU behavior/performance requires native execution. This repository does not claim a real compiler defect or locally verified Grace results.

## Reproduce, reduce, and inspect

```sh
build/a64sentinel --source path/to/case.c --compiler gcc --compiler clang --out out/recheck
python python/regressions.py out/native/seed-1/report.json --executable build/a64sentinel --out out/replay
python python/regressions.py regressions/unsigned_alias.json --executable build/a64sentinel --out out/regressions
python python/report.py out/native --out out/summary.html
```

Add `--cxx` when replaying a C++ file directly. Python replay restores language, flags, CPU, target, runner, seed, and timeout from each saved configuration. By default regression replay expects agreement; `--expect mismatch` verifies a still-reproducing failure. Compiler paths/versions must be installed locally; reports record versions but replay cannot install or enforce identical versions automatically.

On a mismatch, original `source.c`/`.cpp` and `report.json` remain intact. The reducer removes marked independent blocks and recompiles candidates. Each predicate is checked twice against a differing configuration pair with the same original exit codes. `--budget N` bounds predicate attempts (default 48), each allowing two compile/run checks; `--no-minimize` disables reduction. Confirmed reductions produce `minimized.c`/`.cpp`, `minimized-report.json`, and `reduction.json`. Untagged user programs can be replayed and diagnosed but are not reduced. The reducer is block based and does not guarantee globally minimal source.

Each original/minimized configuration directory retains its executable, assembly (`program.s`), and diagnostic logs. Clang also produces LLVM IR (`program.ll`). JSON includes exact compile argument arrays, compiler versions/target triples, timing, stdout/stderr, exit codes, spawn/timeout/truncation states, and all replay settings. Failed diagnostic generation is recorded in its log; IR is only available from recognized Clang drivers.

## Defined behavior and limits

Generated arithmetic uses fixed-width unsigned types with modular overflow, valid shift counts, positive constant divisors, initialized arrays, bounded pointer/index operations, and legal `unsigned char` object access. The checksum observes all arrays. Alignment uses C11 `_Alignas` or C++17 `alignas`. Unsigned byte representation can vary by target endianness; compare configurations on the same target ABI. Generation is stable per seed and generator revision; preserve the source for long-term reproducibility. This is a constrained template generator rather than a complete language fuzzer.

User-provided programs and arbitrary additional compiler flags are outside the generator's defined-behavior guarantee. The process backend uses POSIX process groups or Windows Job Objects to terminate descendants. Captured stream reads are limited to 1 MiB each, but on-disk capture is not size bounded. Execution is not a security sandbox. Windows process paths currently use ANSI APIs. Inconclusive outcomes and compiler crashes need manual diagnosis; equal nonzero exits are reported as runtime errors.

The tests cover deterministic generation, reduction budgets, output/exit/stderr classification, quoting, capture, timeout, real native C/C++ matrices, diagnostic generation, regression replay, and a synthetic compiler fixture that deliberately introduces a mismatch. See [architecture](docs/architecture.md) [regression suite](regressions/README.md), and [local validation](docs/validation.md).
