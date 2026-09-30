# PA25 audit138 performance evidence

The reviewed code tip is `4530fe939e95a45ff6a46d5e53c76807b5bdd352`.
O0 adds necessary constant/ABI semantics and explicit definition/relocation
ownership, with no new optional optimization or speedup claim. The inherited
15% latency/RSS and zero optional text-growth targets remain diagnostics under
[spec section 9](../../spec.md). They do not add exit gates. Mandatory correctness,
coverage, native bounds and finite pipeline work remain unchanged.

## Frozen protocol

[performance.py](performance.py) records binary/input hashes, `-O0`, every
observation and output identity. Each workload/mode has one four-sample A/A
calibration and six ABBA blocks. Compile/link and runtime are timed separately;
paired ratios divide within-block means. All results are checked before timing.
No builds or tests ran concurrently with these measurements. No samples were
removed. The 11 retained runs across handoffs and this audit contain **1624
observations**, including **616 historical** observations. Every recorded input
and binary manifest was rehashed successfully during the audit.

Final A is the entry compiler at `34fe77a3`; final B is the exact reviewed binary:

- `cppgm-before`: `7a52b4b8ec8aedefc0a51596512a78ac0f121728f3612bb55ae6b02a9e9c668d`
- `compiler-code-tip`: `8ec48c55121ff284ee82238203e17a9e5b482c96dc87cdd3d346ba7d981fff3f`

`performance-code-tip/performance.json` is the final comparison. The template
workload demands 4800 specializations over nine TUs in one compiler invocation;
its 0.328 s median dominates startup. Memory/floating compile batches each
contain 64 short invocations and include measurement/process startup; those
compiler ratios are diagnostic. Runtime batches contain three executions with
argc-dependent loop bounds and independently computed checked results. Memory
and floating each execute three million operations per run.

## Exact final binary

| Workload | B compile batch s | Compiler peak KiB A / B | Compile B/A median [range] | Compile A/A s |
|---|---:|---:|---|---|
| templates | 0.32798 | 14744 / 14900 | 1.061 [0.794, 1.304] | 0.27278–0.45814 |
| memory | 0.36814 | 6724 / 6812 | 0.945 [0.818, 1.057] | 0.38512–0.39812 |
| floating | 0.37397 | 6872 / 6932 | 0.952 [0.910, 1.007] | 0.36388–0.40931 |

| Workload | B runtime batch s | Runtime B/A median [range] | Runtime A/A s | Text bytes A / B |
|---|---:|---|---|---:|
| templates | 0.12620 | 0.990 [0.972, 1.070] | 0.12080–0.13186 | 384567 / 384567 |
| memory | 0.15609 | 0.987 [0.935, 1.023] | 0.15315–0.16852 | 521 / 521 |
| floating | 0.14089 | 0.997 [0.920, 1.034] | 0.14102–0.14436 | 340 / 340 |

Runtime peak RSS is 256 KiB. Equivalent executable pairs are byte-identical,
including their instruction bytes. Thus runtime variation cannot demonstrate
an optimization benefit or generated-code regression. The template compiler
median increases 6.1%, with paired blocks spanning 0.794–1.304 and a broad A/A
calibration. This cost is disclosed, not rounded away or claimed as a speedup.
The repaired path performs required O(symbols + relocations) identity mapping,
adjacency construction and deduplicated demand processing; foreign definition
extents are sorted once, O(symbols log symbols). There is no global retry or
optional pass to fund. Avoidable node growth was removed: LowIR symbol metadata
remains 36 bytes and ABI nodes remain 32 bytes. Fixed-width constant work is
bounded; no new common per-expression payload was added.

## Cumulative and intermediate evidence

The stage-base driver is a stub, so it is not a correct equivalent performance
baseline. The earliest enabled driver, preserved as `pa25-135/driver-A`, is a
valid baseline on the fixed workloads. `performance-reviewed-cumulative` compares
it against the packed audit compiler immediately before the final semantic-dump
rendering correction. That last correction affects only an explicit view; the
exact final compiler was measured separately above. This provenance is preserved
rather than labelling the predecessor as the final binary.

| Workload | B compile s | Compile B/A median [range] | B compiler peak KiB | B runtime s | Runtime B/A median [range] | Text bytes |
|---|---:|---|---:|---:|---|---:|
| templates | 0.55791 | 1.028 [0.864, 1.153] | 15180 | 0.22695 | 0.977 [0.924, 0.999] | 384567 |
| memory | 0.69263 | 0.943 [0.880, 1.033] | 6784 | 0.24171 | 0.961 [0.879, 1.041] | 521 |
| floating | 0.65127 | 0.981 [0.865, 1.025] | 6944 | 0.22912 | 0.999 [0.942, 1.063] | 340 |

The packed predecessor's audit-to-entry template median was 1.034, range
0.947–1.452; the earlier ABI-corrected intermediate cumulative median was 1.069,
range 0.934–1.133. The first audit comparison contains a floating compile paired
outlier of 4.290. All are retained, together with the historical 1.230 floating
outlier and all A/A samples. Absolute elapsed times varied substantially between
runs; selecting a favorable run would not establish a benefit.

New wide and statement behavior lacks a correct pre-feature baseline. The
preserved final/final calibrations in [performance136](performance136.md) and
[performance137](performance137.md) measure 200,000 checked wide steps (977 text
bytes) and three million checked calls/destructions (503 text bytes). These are
absolute baselines, not speedup evidence. This audit verifies composition using
wide enum/template/statement cases and normal/early lifetime traces. PA34 owns
self-hosting; O1–O3 policy belongs to later optimization stages.

Acceptance is stage-scoped: required semantic/ABI work is bounded, the observed
latency increase and spread are recorded, equivalent generated programs do not
change, and no unprofitable optional transform was added. The source review
removed name/address reconstruction and unnecessary layout growth. This evidence
supports the checkpoint without inventing a timing threshold or excusing any
of the 27 remaining required behavioral failures.

## Retained evidence

All paths below are relative to `/home/vishvananda/work/private/v4codex/artifacts/`.
Raw observations, binary/input manifests and complete summaries remain in each
JSON; [validation138](validation138.json) records successful manifest verification.

| Record | Observations | SHA256 |
|---|---:|---|
| `pa25-135/performance.json` | 168 | `2791899a05de706bde755483a6d1850b083e72de190226c182c0f5b01e98fc6a` |
| `pa25-136/performance/performance.json` | 168 | `7eb6dab5d43d84e242be70b32860b8951b4fc41ff25d52d97f48fe5b9bc64292` |
| `pa25-136/wide-performance/performance.json` | 56 | `6a39e340677bc2a7d04792848dbc52e76c8d6e6896b7a82a6388e44fae6146b3` |
| `pa25-137/performance/performance.json` | 168 | `eda550ff52b26a0893bac2a2c2361f209614284f45efa733d4070819f7e57df4` |
| `pa25-137/statements-performance/performance.json` | 56 | `7b06b2ce9de444349a8e55e9a855ddc326b01afa779f17c936c01fe7629258fc` |
| `pa25-138/performance-audit/performance.json` | 168 | `8a6fca21022cd941a84884d88ee96bdf69824169bafa0d0539bd23cf68ffa921` |
| `pa25-138/performance-code-tip/performance.json` | 168 | `b2169833bec35577795424b12db29fe456d76bf3ff20510b300045d8d18bb8a7` |
| `pa25-138/performance-cumulative/performance.json` | 168 | `df364f9dfa7d5f23d4da18cdfbffb37eeb0ea340fb5c36d12d82f50905c2344c` |
| `pa25-138/performance-final/performance.json` | 168 | `3479d0ae193b0bf09b970fd86198ae6ebe830da6040db676a2c299fa8a5b0c41` |
| `pa25-138/performance-reviewed-audit/performance.json` | 168 | `dbcab73dc59c2ba81d876d4ffb1cbb635f94399307abc5814006180a4e52ed9a` |
| `pa25-138/performance-reviewed-cumulative/performance.json` | 168 | `494b4504092a322d0881c71e5426463c07a42f4917b5bae13998e6dedf61e8d1` |
