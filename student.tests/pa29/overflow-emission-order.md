# Overflow builtin emission order

The frozen `semantic_overload.cpp` workload compiled successfully but its seed
and self-produced objects differed at O0, O1, O2 and O3. Only the standard
library regex helper `_M_cur_int_value` differed. That helper uses
`__builtin_mul_overflow` and `__builtin_add_overflow`; a single function
returning `__builtin_mul_overflow(a, b, out)` reproduces the mismatch.

`lowering/overflow_builtins.cpp` passed multiple IR-producing calls as arguments
to another IR-producing call. GCC and the student compiler chose opposite
argument evaluation orders, so they assigned different instruction IDs and
emitted different instruction orders. N3485 [expr.call]/8 and
[intro.execution]/15 allow these host evaluation choices. Separate full
expressions now sequence the emissions, preserving the GCC-built seed's
existing order. Arithmetic and source operand evaluation are unchanged.

The normal PA29 run fixture is `run/800-overflow-emission-order.t`, with its
`.t.1` source and implementation/program status sidecars. It checks both
overflow flags and stored results for addition, subtraction and multiplication
on `int` and `__int128`, covering both multiplication lowering paths. Run it
through the existing worker and comparator from the repository root:

```sh
CPPGM_HOST_CXX=g++ perl scripts/run_cpphostinterop_tests_worker.pl \
  dev/cppgm++ check student.tests/pa29/run/800-overflow-emission-order.t
perl scripts/compare_results_common.pl link_program_t ref check \
  student.tests/pa29/run/800-overflow-emission-order.t
```

Runtime checks alone cannot detect this ordering defect: both original
generations compute the same results. Compare the same fixture's LowIR and
objects across generations as the additional reproducibility regression:

```sh
set -eu
mkdir -p obj/overflow-emission-order
for level in 0 1 2 3; do
  for mode in --emit-lowir -c; do
    dev/cppgm++ -O"$level" "$mode" \
      student.tests/pa29/run/800-overflow-emission-order.t.1 \
      -o obj/overflow-emission-order/seed
    pa34/cppgm++-self -O"$level" "$mode" \
      student.tests/pa29/run/800-overflow-emission-order.t.1 \
      -o obj/overflow-emission-order/self
    cmp obj/overflow-emission-order/seed obj/overflow-emission-order/self || exit 1
  done
done
```

This comparison fails with the pre-fix seed/self pair at all four levels.
The full frozen workload is checked the same way with its pinned headers.
PA34's normal inception comparison covers this repository's own compiler
sources at O3; those sources do not exercise the overflow builtins responsible
for the frozen workload's differing output.

Validation with GCC as the host toolchain:

- The original seed/self pair produces different LowIR and objects for the
  fixture at O0/O1/O2/O3, while both generated programs pass its runtime checks.
- The fixed seed, self and inception compilers produce identical fixture LowIR
  and objects at all four levels, and all generated programs pass.
- The fixed seed's fixture output is byte-identical to the original seed's.
- The full frozen workload now produces identical seed/self objects at
  O0/O1/O2/O3.
- `make test-pa29`: 403/403 pass.
- `make test-report`: 5,454/5,454 pass.
- `make inception`: `MATCH cppgm++`.
