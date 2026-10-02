# PA30 vector and packed-operation completion

Implementation203 extends the cumulative compiler. No header names or fixture
contents control behavior. Public builtin spellings are a bounded, immutable
target vocabulary; the canonical selected declaration carries its descriptor ID.
Sources are registered in `dev/frontend_source_sets.mk` for their actual users.
No host/reference compiler or assembler participates in implementation.

## Ownership and data flow

- `semantic/expression.cpp`, `query_operator.cpp`, `assignment_operators.cpp`
  and `operators.cpp` own vector lane types, cv, value categories, dependent
  assignments and unary operations. Query and evaluated intrinsic calls share
  the immediate constraint owner. Bounds are checked after substitution;
  querying a signature does not demand a body.
- The ordinary constant storage graph owns vector subobjects, including
  constant and substituted lane queries. Vector selectors cannot be mistaken
  for class member IDs or mutable-member flags.
- `semantic/vector_shuffle.cpp` interns a signature keyed by canonical function
  type, including both source types, mask and arity. This TU-owned cache has no
  declaration-insertion invalidation: its keys contain only immutable types.
- `lowering/values.cpp` captures vector values before subsequent operand effects.
  Plain values copy their typed representation; volatile values read all lanes
  explicitly. Subscripts retain the original location and only read/write the
  selected lane. Representation casts and bit-casts use captured vector values.
  Volatile destinations use explicit lane stores. No frontend nodes are cloned.
- Fixed packed integer operations lower through scalar typed LowIR: widened
  arithmetic, saturation, lane interleave and a safe shift count. Variable
  packed counts consume all low 64 bits; out-of-range shifts produce zero or
  the sign fill required by the instruction. A signed multiply-add retains
  its modulo-32-bit result. Shuffles snapshot all operands once, mask indices,
  and select an in-range source lane; wide vectors use a generated loop.
- Floating/target-state operations use a finite target-intrinsic LowIR extension
  because approximation, MXCSR, NaNs, upper-lane preservation and non-temporal
  memory effects cannot be replaced by arbitrary scalar arithmetic. Instruction
  selection and direct encoding consume the same descriptor ID, never a
  semantic name search. The normal native backend and ELF writer remain shared.

## Serialized target-intrinsic boundary

`x86 ID, RECORD, PREDICATE` has no SSA result. `ID` identifies a native entry in
`support/x86_builtins.cpp` (append-only vocabulary), and `RECORD` is a pointer to
64 bytes of expression-local storage: output starts at 0, inputs at 16/32/48.
The descriptor fixes every field's type, width, order and effects. It is an
architectural operation, not an arbitrary machine opcode or register request.
Only the encoder chooses XMM14/15 and R10/R11. Masked stores temporarily use and
restore RDI. Results initialize every byte belonging to their declared type.

`PREDICATE` is the selected comparison immediate (0–31), or unsigned -1 when
there is no recorded immediate. A known literal emits one comparison. The
bounded general record form has at most 32 alternatives; no search or variable
code growth occurs. Frontend calls still enforce the builtin's immediate rules.
External LowIR validation rejects invalid descriptor IDs, non-native lane
recipes, non-pointer records and misplaced/out-of-range predicates. Callers of
this explicit IR operation provide the descriptor's initialized input fields
and valid memory operands, just as for ordinary loads/stores.

The reader, writer, validator, MIR dump and encoder all support the operation.
The production object path constructs it directly. Serialized reconstruction
is an explicit personal inspection test, not transport between phases. PA8's
handout allows later producers and consumers to extend this interface.

## Work, ownership and budgets

Builtin lookup scans only a fixed target vocabulary once per unresolved name;
completed declarations then use ordinary indexed scope lookup. Canonical
signature lookup is O(1) average. No global retry, mutable process cache, token
replay, tree copy, or template-body demand was introduced.

Fixed packed inputs are 8 or 16 bytes: lane expansion is bounded by 16 lanes,
with constant work per lane. Ordinary vector loops unroll at most eight lanes;
larger vector operations emit one loop. Native records use 64 bytes per
expression; their temporary storage belongs to the containing function and is
released with function lowering/native scratch. The comparison fallback is
bounded by 32 alternatives. Overall work and output growth are linear in source
operations plus consumed/generated IR. Existing forced-inline work/depth limits
remain unchanged. Existing phase time, RSS and IR/native counters observe the
new work without extra analyses.

These are correctness additions at O0, with no optional optimizer or speedup
claim. The mandatory stage limit is 45 seconds per hosted compile. Performance
records compare equivalent correct programs separately from new cases rejected
by entry. Historical diagnostic percentage/zero-growth targets do not override
spec §9. PA31 hosted execution, PA32/33 optimization and PA34 self-hosting remain
owned by their stages; this does not waive current-stage correctness.

## Semantic references

GNU vector types, scalar lane operations, subscripting, casts and shuffle masks
follow the [GCC vector extension documentation](https://gcc.gnu.org/onlinedocs/gcc/Vector-Extensions.html).
Target signatures are checked against the installed GCC 15 intrinsic headers
and [GCC x86 builtins](https://gcc.gnu.org/onlinedocs/gcc/x86-Built-in-Functions.html).
Packed saturation, count handling, floating status/rounding and encodings follow
the [Intel instruction reference](https://www.intel.com/content/www/us/en/content-details/851063/intel-64-and-ia-32-architectures-software-developer-s-manual-combined-volumes-2a-2b-2c-and-2d-instruction-set-reference-a-z.html).
Hardware differential controls supplement those contracts; compiler agreement
is not used to revise any reference. All supplied fixture/reference files and
comparison rules remain unchanged.
