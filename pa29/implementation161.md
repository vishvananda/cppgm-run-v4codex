# PA29 implementation161: atomic storage and intrinsic families

Entry `7f0a2c4b` had 323/403 passing fixtures. The atomic ownership group extends
GNU fetch/add into C11 atomics, GNU scalar/generic atomics, legacy sync operations,
feature probes, ordinary scalar atomic accesses, class representation transport
and intrinsic exception facts. Assembly constraint parsing remains a separate
unfinished owner. Course inputs, references and comparison rules are unchanged.

## Ownership, data flow and complexity

- `syntax/declarator.cpp` parses `_Atomic(type-id)` once into an `AtomicType`
  node. The normal semantic type builder validates unqualified, non-array,
  non-reference, trivially copyable operands. Dependent operands use the existing
  canonical type/frame substitution cache, including expected substitution failure.
- `semantic/model.cpp` interns atomic storage in qualifier bit 4, independently
  of const/volatile. Removing cv preserves atomic identity; value conversion
  explicitly removes atomic. Function parameter signatures, pointer qualification,
  partial specialization and ABI naming retain that distinction. Layout rounds
  small atomic representations to a power of two and supplies width alignment;
  literal-type checks continue to distinguish volatile subobjects.
- `support/atomic_builtins.h` owns the bounded immutable vocabulary and packed
  operation/form/result descriptor. Probes use that registry. The semantic owner
  caches selected function signatures by descriptor and canonical pointee type,
  after arity/type/cv checks. Selected declarations retain argument conversions
  and nonthrowing facts; lowering performs no lookup or overload resolution.
- `lowering/atomic_builtins.cpp`, `atomic_storage.cpp` and `atomic_runtime.cpp`
  consume those facts and existing layout facts. Integer, pointer and scalar
  representation operations use existing typed LowIR opcodes. Bitwise operations
  and ordinary compound updates each emit one CAS loop; source operands are
  evaluated before the loop. Expected storage updates only on CAS failure.
- Class results use the semantic destination/temporary identity, including
  elision. Padded atomic representations zero their extra bytes and copy only the
  ordinary payload into/out of value and expected objects. No four-byte store can
  overrun a three-byte result. Generic GNU representations without a safe inline
  primitive, including 16-byte classes with only 8-byte type alignment, use the
  documented libatomic generic ABI. Runtime helper symbols have one identity per
  linked program and retain nonthrowing signatures in serialized LowIR.
- Atomic reads cannot become constexpr reads. Scalar writes, increments and
  compound assignments remain atomic. Generated transfers of atomic members do
  not coalesce their reads/writes into ordinary byte copies. The underlying
  native selector/object writer and explicit external-LowIR adapters are shared.

The vocabulary has a fixed bound; each call examines its supplied arguments and
uses average O(1) canonical signature lookup. Type/substitution storage is TU-owned
and released with the semantic graph. Per-function values, slots and CAS blocks
are released with lowering. Each operation generates constant work/IR, except
payload copying and generic runtime calls proportional to actual object width.
One emitted CAS loop has unbounded *runtime contention*, not unbounded compiler
search. No optional optimization or new global pass is introduced. Existing
constant-evaluation, native alignment/frame and course timeout limits remain.

## Semantic contracts

The [GNU atomic contract](https://gcc.gnu.org/onlinedocs/gcc/_005f_005fatomic-Builtins.html)
permits sequentially consistent implementations of weaker orders and dynamic
orders, plus a strong implementation of weak compare-exchange. All order operands
still evaluate once. GNU pointer arithmetic counts bytes; C11 pointer arithmetic
scales by pointee size. GNU generic operations may call libatomic when inline
instructions cannot safely implement the representation. Lock-free queries report
only guaranteed widths; `always_lock_free` requires a constant size and leaves
its optional pointer unevaluated.

The [legacy sync contract](https://gcc.gnu.org/onlinedocs/gcc/_005f_005fsync-Builtins.html)
provides the return-value and synchronization requirements for sync CAS, fetch,
lock and fence operations. The
[Clang extension documentation](https://clang.llvm.org/docs/LanguageExtensions.html#c11-atomic-builtins)
identifies the C11 builtin surface; `pa29/README.md` specifically requires atomic
literal subobjects and nonthrowing intrinsic lowering. `pa8/lowir.md` owns the
existing atomic IR contract. No reference correction was needed.

## Validation and handoff scope

Explicit controls live in `student.tests/pa29/controls161/`; the harness runs
runtime, concurrency, overload/type identity, padding, generic fallback and
rejection checks. `inspection161.py` validates/roundtrips emitted LowIR, executes
it through a personal adapter calling the production object writer, checks native
locked instructions and ABI names, and compares telemetry/non-telemetry objects.
The adapter is test-only; source compilation never passes through textual IR.

The final plan and performance report record the command outcomes and frozen
measurements. The atomic fixture group is the implementation boundary; remaining
assembly cases need parsed constraints, operands and clobber/effect recipes,
which do not follow from selecting an atomic builtin declaration. Unfinished
syntax/template/ABI groups and inherited independent review questions remain in
`plan.md`; this handoff does not certify PA29 or waive its independent audit.
