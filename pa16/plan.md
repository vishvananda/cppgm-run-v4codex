# PA16 implementation — loop 38

Stage base commit: `438d56b164600f4fa19d25dcb5f09a76e2a79776`
Last reviewed commit: `438d56b164600f4fa19d25dcb5f09a76e2a79776`

Target: **PA16 full-stage**. Entry `cc842756`: **63/154, 91 failures**,
clean. Previous turn: progress (validated scalar handoff). Current intermediate
result: **78/154, 76 failures**, 15 entry failures fixed, no lost passes.
Independent stage review remains pending; implementation handoff is not advancement.

## Design and completed behavior

| Owner | Data flow, complexity and lifetime | Validation |
|---|---|---|
| Inherited `constant_execution`, `constant_statements`, `constant_floating`, `constant_array` | Checked graph -> typed scalar activation/frame and target floating payload; 512 active calls / 1,000,000 executed steps per root; array child indexes use O(explicit items) setup and O(log items) reads. TU caches and transient frames retain their existing owners. | Loop 37: 18 existing failures fixed, 40 native / 23 rejection controls; frozen evidence in [performance.md](performance.md). |
| `semantic/static_initialization` | Completed declaration/initializer/constructor facts -> constant classification, memoized by entity and (plan, local-context). Flat TU indexes; each plan visits its children once per context. No textual key or lowering-time lookup by spelling. | Scalar, reference, array, aggregate, factory and template-local static course cases. |
| `lowering/local_static` | Declaration identity -> persistent global, first-use guard, initializer actions and one exit callback. Registration follows successful initialization; reference-owned temporaries have indexed callback edges and conditional lifetime guards. Function transient state releases as before. | Explicit storage controls check repeated first use, declarations/overloads/specializations, references to parameters, lifetime extension, reverse callbacks and arrays. Test-owned atexit runtime executes callbacks because the supplied freestanding backend has no libc atexit. |
| `lowering/constant_array` | Validated constexpr plan -> typed data items -> flat structural interning by size/alignment, item kinds/types/payloads, symbol IDs/addends. O(emitted data) hash/equality; duplicate tentative data is released. Every automatic array retains its own slot and one copy, including arrays above 32 bytes. | Shared literals across template instantiations; distinct addresses, large omitted ranges, floating signed zero and relocation addends. |

The production path remains the shared graph and typed LowIR. No reference/test
changes, text transport, host compilation or additional optional optimizer.
Static data cannot contain addresses of automatic variables or unbound reference
parameters; such initializers use first-use execution.

## Remaining implementation / concrete boundary

- **Object/address execution:** typed object roots, subobject paths, activation
  lifetimes and bounds; constructor/base/member execution, references, callable
  values and conversions. Scalar payloads cannot represent those semantics.
- **Declaration validity:** literal-type and constexpr declaration facts,
  dependent-template validity, missing member initialization.
- **Exception expressions:** selected calls/defaults, constructors and destructors
  must feed deferred `noexcept` queries.
- **Storage depending on the evaluator:** constexpr class/static-pointer values,
  aggregate/member projections, static address-producing calls and their demands.
  These require the object/address engine above, not more guard lowering.
- **Ordinary automatic constant arrays:** PA16 README requires a copy, while 16
  inherited PA10–15 fixtures compare against element stores with identical tool
  flags. A trial implemented this policy and passed native controls but failed
  those earlier comparisons (`prior-1.log`). It was reverted to preserve required
  earlier coverage; constexpr array copies/interner remain. This is an unresolved
  contract/implementation requirement, not a waived performance gate. Reducer:
  `int f(){int a[2]={1,2};return a[0];}`; both stores and a copy have correct C++
  behavior, so the permitted C++-miscompilation reference exception is unproven.

The current coherent group is persistent local storage and constexpr-array data
ownership. Further storage-related evaluator failures require a new object/address
value domain and call/lifetime keys. The ordinary-array conflict requires contract
review before changing earlier or current comparison obligations. Neither is
classified as an optional architecture improvement.

## Performance and validation

Stage: PA16 / O0, required semantic/storage work, no optional transform. The
historical PA15 32-byte copy heuristic is not a gate: PA16 explicitly requires
constexpr automatic-array copies. Earlier measurements remain in performance.md;
new latency/RSS, runtime/text and scaling evidence is pending in this increment.
No stage-mandated numeric performance ceiling is introduced or relaxed.

Intermediate `make test-pa16`: **78/154**; `make test-report-through-pa15`:
**2112/2112**. File audit passes with the three inherited header warnings.
Scalar/floating controls: **40 native + 23 rejection**; storage controls expanded
from 22 cases. Final serial reports, frozen performance and clean commit are
required before handing off.

## Handoff ledger / independent review

| Increment | Disposition |
|---|---|
| `d7594d63`, `482c6b44`, `7484f22b`, `cc842756` | Loop 37 scalar group and validated performance handoff; review pending. |
| Loop 38 storage increment | Local static identity, initialization, guards, callbacks and constant-data interning; validation in progress. |

Independent review must retrace accumulated scalar activation/cache identity,
target floating semantics, array-range ownership, completed static classification,
callback lifetime edges, data interning and stage-scoped performance evidence.
These review obligations are distinct from the unfinished implementation and
ordinary-array contract question above. Both review markers remain unchanged.
