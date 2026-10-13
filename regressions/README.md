# Regression suite

`unsigned_alias.json` is a portable configuration recipe for the bounded, aligned unsigned aliasing case in `cases/`. It is baseline coverage, not a claim of a discovered compiler defect.

```sh
python python/regressions.py regressions/unsigned_alias.json --executable build/a64sentinel --out out/regression-check
```

Promote a discovered, reviewed failure by copying its minimized source and `minimized-report.json` into this directory, adjusting the report's relative `source` and compiler executable paths. Preserve compiler versions, flags, and observed logs. The regression runner expects agreement after a fix. Before a fix, use `--expect mismatch` to verify reproduction. Use a fresh output directory each time.

The automated integration suite uses a synthetic compiler to check failure reduction and replay. Synthetic cases are explicitly labeled and never represented as real compiler bugs.
