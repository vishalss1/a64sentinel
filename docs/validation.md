# Validation record

Validated on 2026-10-13. These are local results, not claimed CI or Grace hardware results.

| Environment | Checks | Result |
| --- | --- | --- |
| Windows x86_64, Clang 22.1.4 framework build | CMake/Ninja, 11 GoogleTest cases, Python integration | 12/12 CTest tests passed |
| Windows, GCC 16.1.0 and Clang 22.1.4 | Three C and three C++ seeds, each at O0/O2/O3 with both compilers | All 36 configurations agreed |
| Windows synthetic compiler fixture | Confirmed mismatch, reduction from 12 blocks to one required block, assembly/IR artifacts, replay, zero-budget and relative-path checks | Passed; fixture deliberately changes behavior |
| Windows portable regression recipe | Unsigned alias case, six GCC/Clang configurations, repeat executions | Pass |
| Ubuntu 26.04 WSL x86_64, GCC 15.2.0 | Direct C++17 build, 11 GoogleTest cases using fetched GoogleTest sources | 11/11 passed |
| Ubuntu WSL, GCC 15.2.0 | Four generated C seeds at O0/O2/O3 | All 12 configurations agreed |
| Ubuntu WSL, GCC 15.2.0 | 12 generated C and 12 C++ programs at O2 with ASan/UBSan and nonrecovering diagnostics | All 24 passed with empty sanitizer stderr |

Windows reproduction commands:

```powershell
cmake -S . -B build -G Ninja -DCMAKE_CXX_COMPILER=clang++
cmake --build build
ctest --test-dir build --output-on-failure
python python/regressions.py regressions/unsigned_alias.json --executable build/a64sentinel.exe --out out/regression-check
```

The Ubuntu environment had GCC and Python but no CMake/Clang. Linux binaries were compiled directly with `g++ -std=c++17 -O1 -Wall -Wextra -Wpedantic -pthread`, using the same framework sources and GoogleTest sources fetched by the Windows CMake build. The POSIX checks were rerun after launch-status error handling changed. The generator did not change after its sanitizer audit.

The checked-in GitHub workflow configures native GCC/Clang builds, Linux sanitizer audits, and AArch64 GCC/Clang differential execution under QEMU with `neoverse-v2` targeting. Those CI jobs have not been executed from this workspace. An attempt to retrieve WSL packages was stopped after package retrieval stalled; no AArch64 toolchain/QEMU was installed. Native AArch64/Grace behavior and emulated ARM64 execution remain unverified locally.

Reduction is limited to independently removable generated blocks. The framework does not prove arbitrary input programs free of undefined behavior and does not claim any real compiler defect from these checks. See the README for execution and portability limits.
