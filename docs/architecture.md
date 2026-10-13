# Architecture

`sentinel` is a reusable C++ library. `a64sentinel` is a sequential campaign CLI. Python consumes versioned JSON reports without bindings or dependencies.

- `generator`: SplitMix64 seed stream, stable fixed-width constants, initialized state, independently removable statement blocks, observable checksum.
- `process`: argument-vector execution, explicit working directory, captured binary streams, elapsed time, process-tree deadlines, output truncation detection.
- `testing`: compiler target discovery, C/C++ standard selection, per-configuration compilation and execution, matrix classification.
- `minimize`: coarse-to-fine deletion of contiguous marked blocks, bounded predicate evaluations, baseline confirmation. The CLI supplies a repeat-checked differential predicate.
- `report`: JSON serialization, assembly and Clang IR generation, diagnostic logs. Raw bytes are mapped individually to JSON U+0000..U+00FF for reversible byte-level comparison.
- `options`: validates matrix, runner, target, seed/count ranges and numeric bounds.

## Classification

A runtime mismatch requires at least two successfully compiled, started, completed executions with untruncated logs. Any difference in exit status, stdout, or stderr is suspicious. An unsuccessful compile returns `compile_error`; execution timeouts, spawn failures, or truncated streams return `inconclusive`. If behavior agrees with a nonzero exit, classification is `runtime_error`. Only agreement with zero exits is `pass`.

## Reduction contract

Generated blocks mutate preinitialized state without introducing declarations needed by subsequent blocks. Removing any subset preserves syntactic validity and defined operations. A candidate is interesting only when the originally differing configuration pair still differs twice and preserves each original exit code. The final reduced source is rerun across the entire matrix and receives its own observed status. An unconfirmed baseline has no minimized artifact. Reduction is intentionally conservative and does not parse arbitrary C/C++.

## Reproducibility

Each report stores source location relative to the report, exact compiler argument arrays, per-compiler flags and runner vectors, versions, compiler triples, seed, language, target, CPU, and deadlines. Original and minimized artifacts use separate directories. Replay rebuilds each configuration twice and compares observed behavior, treating local compile failures and instability as inconclusive. Source files and executable toolchains, rather than seeds alone, define durable reproductions.

## Extension points

Add independently safe generator families to the statement blocks. Add independent configuration runs through the `test` library API. A more general reducer can implement the existing predicate contract, but must validate that candidate sources avoid undefined behavior before treating differences as defects. New report fields should be additive or bump `schema_version` if the contract changes.
