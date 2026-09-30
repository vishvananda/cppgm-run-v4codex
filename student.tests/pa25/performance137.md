# PA25 implementation137 performance evidence

O0 scope: scoped statement values, template queries, control transfers and lifetimes. No new optional optimization pass or speedup claim. The existing destination copy-elision policy now recognizes the new result form. Optional transform work/growth budgets remain zero; required semantic work is linear in source, consumed lifetime edges and emitted IR. Sparse guards are emitted only where a jump can skip construction. Cache keys and release points are described in [ownership](statement-expressions137.md).

Frozen A is entry `11aee2da`; B is production `2705e57b`. Compiler sizes: 3262856 / 3284472 bytes (+0.66%). SHA256:

- A: `a30591cca86ea42f2a65ba3cc00e2f5714ff11055913739f4e30c9e0bb4ee417`
- B: `7a52b4b8ec8aedefc0a51596512a78ac0f121728f3612bb55ae6b02a9e9c668d`

[performance.py](performance.py) preserves flags (`-O0`), input/binary hashes and every observation. Each workload has an A/A block and six ABBA blocks; compiler and executable timing are separate. No builds or tests ran concurrently. All executable results are checked using runtime-dependent inputs. Equivalent A/B executable pairs are byte-identical. No samples were removed.

| Workload | B compile batch s | Peak KiB A / B | Paired B/A median [range] | A/A batch range s |
|---|---:|---:|---|---|
| templates | 0.23988 | 15588 / 15704 | 1.011 [0.986, 1.019] | 0.23191–0.24997 |
| memory | 0.37111 | 6712 / 6836 | 0.976 [0.944, 0.987] | 0.38240–0.41468 |
| floating | 0.36434 | 6836 / 6936 | 0.965 [0.938, 1.230] | 0.37185–0.44071 |

The template workload contains 4800 demanded specializations across nine TUs and takes about 0.24 s per compiler invocation. This is the compiler-latency acceptance workload that dominates startup. Memory/floating batches contain 64 short compiler invocations; their process/measurement startup costs remain included, so those compiler ratios are diagnostic rather than speedup evidence. RSS increases are under 1.9%.

| Workload | B runtime batch s (3 runs) | Paired B/A median [range] | Text bytes A / B | A/A runtime range s |
|---|---:|---|---:|---|
| templates | 0.17852 | 0.983 [0.859, 1.048] | 384567 / 384567 | 0.14192–0.20682 |
| memory | 0.15069 | 0.997 [0.922, 1.126] | 521 / 521 | 0.15116–0.15317 |
| floating | 0.13823 | 1.000 [0.994, 1.005] | 340 / 340 | 0.13828–0.13855 |

The template runtime and small compiler timings show substantial environmental spread; byte-identical executables preclude attributing runtime differences to generated-code changes. The floating compiler includes a 1.230 paired outlier, retained along with its full A/A calibration. None of these observations is presented as an optimization benefit.

A rejects statement expressions, so it is not a correct equivalent baseline for the new surface. A separate final/final calibration executes three million runtime-driven calls and scope destructions, with normal and early returns checked against a Python-computed sum and exact destructor count.
The 64-compile batch median is 0.38425 s, peak compiler RSS 6832 KiB; runtime median is 0.12588 s per three executions; text is 503 bytes. Compiler final/final paired median 1.002, range [0.960, 1.043]; runtime 0.995, range [0.918, 1.004]. A/A ranges: compiler 0.37656–0.43385 s, runtime 0.12751–0.13762 s. This records complete invocation cost, not an improvement claim. Executable RSS observations are 256 KiB.

Stage acceptance: required semantics have bounded work; the equivalent template compiler median is 1.011 with all paired blocks below 1.020, and equivalent generated text does not change. There is no demonstrated avoidable regression or unprofitable optional transform. Inherited 15% diagnostic targets do not become extra exit gates; all mandated correctness, coverage and complexity requirements remain. PA34 owns self-hosting. Existing benchmark inputs and prior measurements remain available.

Artifacts: `/home/vishvananda/work/private/v4codex/artifacts/pa25-137/`.

| Evidence | SHA256 |
|---|---|
| `performance/performance.json` | `eda550ff52b26a0893bac2a2c2361f209614284f45efa733d4070819f7e57df4` |
| `statements-performance/performance.json` | `7b06b2ce9de444349a8e55e9a855ddc326b01afa779f17c936c01fe7629258fc` |
| `trace.jsonl` | `46dafa28e2421bc1d91b2f528bdcd2f40f0184392c00cd63770d1fcdda4dcfe6` |
