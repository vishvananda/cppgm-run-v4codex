# PA13 personal checks and evidence

Run explicitly from the repository root:

```sh
python3 student.tests/pa13/check_virtual_semantics.py
python3 student.tests/pa13/check_native.py
python3 student.tests/pa13/check_ir.py
python3 student.tests/pa13/check_course.py dev/cppgm++
python3 student.tests/pa13/check_literal_storage.py
python3 student.tests/pa13/audit_check.py
python3 student.tests/pa13/check_linkage.py
```

The first four scripts accept an alternate compiler path, including the actual
ASan/UBSan compiler built with `student.tests/pa10/build_sanitizer.py`. Native
checks use the supplied PA8 backend after this compiler generates and validates
its own LowIR. All execution inputs have defined behavior and checked outcomes.

The five `audit-*.cpp` reducers cover heap-array vpointer initialization, explicit
virtual destruction and qualification, global versus class deallocation,
virtual conversion slots, and virtual assignment/call operators. `audit_check.py`
also checks signature refinement ownership and array IR growth independently
of runtime extent. [Reference proof](../../pa13/reference-corrections.md).

The benchmark invocation is:

```sh
python3 student.tests/pa13/benchmark.py \
  "$RALPH_ARTIFACT_DIR/pa13/compiler-A" "$RALPH_ARTIFACT_DIR/pa13/compiler-B" \
  "$RALPH_ARTIFACT_DIR/pa13/performance" student.tests/pa13/performance.json
```

Run it without concurrent compiler builds or execution tests. It freezes input,
binary, flag and output identities, records warmups and every A/A/ABBA sample,
and measures new virtual semantics absolutely rather than comparing them to an
incorrect stage-entry result. See [the report](performance.md) and [implementation
trace](../../pa13/implementation.md).

The independent final campaign uses the frozen audit-entry/final binaries:

```sh
python3 student.tests/pa13/audit_benchmark.py \
  "$RALPH_ARTIFACT_DIR/pa13-final-audit/compiler-A" \
  "$RALPH_ARTIFACT_DIR/pa13-final-audit/compiler-B" \
  "$RALPH_ARTIFACT_DIR/pa13-final-audit/performance" \
  student.tests/pa13/final-audit-performance.json
python3 student.tests/pa13/audit_noise.py
python3 student.tests/pa13/audit_scaling.py
python3 student.tests/pa13/audit_verify.py
python3 student.tests/pa13/verify_performance.py
```

Run the campaigns sequentially without concurrent builds/tests. Their fixed
inputs reuse the original frozen corpus; the new executable behavior families
are measured only on the corrected compiler. The
[final review](../../pa13/final-audit-performance.md) retains all measurements,
paired results and stage-scoped acceptance.
