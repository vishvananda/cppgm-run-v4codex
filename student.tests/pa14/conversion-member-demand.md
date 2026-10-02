# Conversion member demand

The frozen `semantic_overload.cpp` workload in `~/cppgm-extended`, using its
frozen `stable/include/` closure, failed with `invalid operand conversion`.
The first failure was `if (storage)` in `template_model.h:78`, where `storage`
is a reference to a `std::shared_ptr` specialization whose conversion members
had not yet been instantiated.

`100-conversion-member-demand.t` reduces this to a class template with an
explicit Boolean conversion. The function using its reference is defined
before any object requires the specialization to be complete. The original
compiler rejects that function; the fixed compiler executes both the false
and true cases correctly.

`Analyzer::conversion_candidates` now completes the source class before
reading its conversion members, using the existing template-definition mode
guard and completion machinery. This also covers inherited conversions and
other callers of conversion lookup. N3485 [temp.inst]/1,5 requires class
instantiation when its member or base lists affect expression semantics;
[conv]/4 permits the explicit Boolean conversion in an `if` condition.
See the checked-in [standard](../../doc/n3485.txt).

Run the reduced regression explicitly from the repository root:

```sh
make -C pa14 check TEST=../student.tests/pa14/100-conversion-member-demand.t
make -C pa14 check TEST=../student.tests/pa14/100-conversion-member-demand.t \
  CPPGM_TEST_APP=../pa34/cppgm++-self
```

The fixture uses the ordinary PA14 LowIR and exit-status sidecars, so it can
move into `pa14/tests/spec/` without a custom runner. Its `main` also checks
both conversion results when compiled and run as a native program.
The sidecars were generated with the supplied `cppgm++-ref` through
`make -C pa14 ref-test TEST=../student.tests/pa14/100-conversion-member-demand.t`.

Reproduce the frozen workload without using live project headers:

```sh
dev/cppgm++ -O1 \
  -I "$HOME/cppgm-extended/benchmarks/self_compile/stable/include" \
  -c "$HOME/cppgm-extended/benchmarks/self_compile/stable/semantic_overload.cpp" \
  -o /tmp/semantic-overload.o
```

Validation with GCC as the host toolchain:

- The original compiler rejects the reduced fixture with `invalid operand conversion`.
- The PA14 fixture check passes with the seed and self-built compilers.
- Native execution passes at O0/O1/O3 with seed, self and inception compilers.
- The frozen workload compiles at O1 with seed and self-built compilers; its
  source and all 51 headers match `PERF_EPOCH.json`.
- `make test-pa14`: 314/314 pass.
- `make test-report`: 5,454/5,454 pass.
- `make inception`: `MATCH cppgm++`.
