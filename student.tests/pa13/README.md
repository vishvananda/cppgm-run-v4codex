# PA13 personal checks and evidence

Run explicitly from the repository root:

```sh
python3 student.tests/pa13/check_virtual_semantics.py
python3 student.tests/pa13/check_native.py
python3 student.tests/pa13/check_ir.py
python3 student.tests/pa13/check_course.py dev/cppgm++
python3 student.tests/pa13/check_literal_storage.py
```

The first four scripts accept an alternate compiler path, including the actual
ASan/UBSan compiler built with `student.tests/pa10/build_sanitizer.py`. Native
checks use the supplied PA8 backend after this compiler generates and validates
its own LowIR. All execution inputs have defined behavior and checked outcomes.

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
