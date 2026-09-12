# PA14 personal validation and performance

Run from the repository root:

```sh
python3 student.tests/pa14/check_functions.py
python3 student.tests/pa14/verify_performance.py
```

`check_functions.py` compiles all six local `.cpp` sources with LowIR validation,
then runs the generated programs through PA8's supplied native backend. They
cover specialization demand/identity, compatible declarations, lazy class
completion, calls/operators/defaults/references, static function addresses,
overloaded argument deduction, ordinary/template overload sets, local hiding
and converting class references. The compiler implements the LowIR itself.

`check_sanitizers.py RELEASE SANITIZED` runs all 314 course sources and six
personal sources through both frozen compilers. It requires equal status,
byte-identical successful LowIR and no ASan/UBSan report. Rejection parity for
incomplete-stage inputs is a memory-safety check, not a course correctness pass.
The final campaign checked 320 inputs with Clang's address and undefined behavior
sanitizers, leak detection and halt-on-error enabled.

The [performance review](../../pa14/performance.md) explains the raw JSON and
acceptance. `benchmark.py A B WORK OUT` uses the fixed PA10 compiler/native
corpus plus PA14 template workloads. New semantic cases are measured only on
the working compiler. `graph_read_benchmark.py B C WORK OUT` isolates the source
graph read change using the frozen preliminary corpus. Inputs, outputs, binaries,
flags, hashes, wall/RSS/context-switch observations, A/A calibration and ABBA
pairings are retained. The verifier checks all artifacts and pairings, including
historical measurements; it does not require a timing threshold.

Frozen binaries and generated inputs/outputs live under
`$RALPH_ARTIFACT_DIR/pa14-entry/` and `pa14-measurements/`. Raw JSON contains their
absolute paths and SHA-256 identities. Those external artifacts are required to
rerun hash verification; they are not compiler implementation inputs. No generated
binaries, objects, test logs or `.my*` files are committed.
