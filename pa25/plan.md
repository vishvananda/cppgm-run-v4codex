# PA25 implementation135 handoff

Stage base commit: fee6ad9076ff35c5272526e1c4c4df235fbf3bfe
Last reviewed commit: fee6ad9076ff35c5272526e1c4c4df235fbf3bfe

Target remains **PA25 full-stage**. Entry: clean, **0/101** (101 stub failures).
Prior goal turn had no implementation evidence; fresh inspection established
that baseline. This is a completed driver behavior group, **not stage approval**.

## Design/spec alignment and completed group

`toolchain/driver` owns options and one-source-TU compilation. Shared
`lowering::build_program` streams tokens into the existing typed semantic graph
and LowIR. `native::compile_image` selects/releases one function's MIR at a time
and encodes native bytes. `toolchain/object` persists bytes, native ABI exports,
local IDs, aliases, roles and relocations in a versioned x86-64 binary format.
There is no source/LowIR serialization between production phases, host compiler,
assembler, linker or reference delegation. Frontend graphs die per source TU;
linking consumes/releases each input object after importing its native facts.

`toolchain/linker` owns indexed strong/weak resolution, local isolation, alias
addresses, discarded-definition fixup ownership, lifecycle order and relocation.
ABI spellings are boundary keys, interned once into the flat link index. Ordinary
semantic decisions remain identity-based in their existing owners. `elf_input`
imports allocated ELF sections and direct, absolute, PC-relative and GOT-relative
relocations; GOT entries preserve the instruction's actual indirect semantics.
Startup and output reuse the PA24 encoder/ELF writer. No optional optimization.

Completed: compile/direct/mixed parity, source-independent objects, required
include/target/library options, command macros/redefinition diagnostics, strong
and weak exports, internal objects, data/function pointers, constructor aliases,
foreign helper calls/data, shared TLS and backend helper identities. Follow-up
controls cover discarded weak references, malformed objects, physical include
paths after `#line`, Clang GOT relocations and 4096-aligned foreign functions.
Complexity is O(bytes + (symbols + relocations) log symbols), storage O(input +
output); there are no global retries or fixed-point transforms.

## Remaining implementation (requirements retained)

| Owner/group | Required failures | Data flow and next work |
|---|---:|---|
| Native C++ runtime | 12 | Recorded RTTI/allocation/pure-virtual roles -> actual native support definitions; dynamic-cast must consume hierarchy data |
| Source EH/runtime | 14 | Catch clauses, payload type/lifetime and function-try boundaries -> matching/unwind/runtime; PA24's scalar LowIR EH is insufficient |
| Canonical wide source types | 6 | Token/type recognition, arithmetic conversion and source lowering -> existing native i128 operations |
| GNU statement expressions | 3 | Parser/typed statement result/lifetime -> lowering, including enclosing return |
| Floating builtins | 1 | Semantic builtin identity for nanl/isnan -> typed constant/lowering |
| Floating calculator | 1 | Five one-ULP f64 differences in 1M outputs; execution/oracle ownership still to establish, references preserved |

Also observed outside required fixtures: `student.tests/pa25/unused-dependent-local.cc`
is rejected by inherited frontend `--emit-lowir`; unused dependent body checking
remains implementation work. Diagnostics/case identities are in
[remaining135](../student.tests/pa25/remaining135.json).

The boundary is substantive: all driver/ELF issues found in the controls are
resolved. Further required progress now needs distinct language/runtime owners,
including typed C++ exception matching and RTTI/allocation support; resolving a
symbol to a placeholder would be incorrect. Wide types and statement expressions
require frontend construction/conversion work, not another link-path extension.

## Validation/performance and ledger

- Final `make test-pa25`: **64/101**, 37 failures; every original case retained.
- `make test-report-through-pa24`: **4152/4152**, all earlier stages pass; focused
  property checks pass. A concurrent intermediate run contaminated summary
  files; authoritative reports were rerun sequentially.
- Explicit `python3 student.tests/pa25/driver.py`: **61 checks pass**.
- File audit passes with four inherited header warnings; `git diff --check` clean.
- [Performance135](../student.tests/pa25/performance135.md): frozen A/B, A/A and six
  ABBA blocks, wall/RSS/runtime/text together. Paired compiler medians 0.997–1.021;
  all three executable pairs byte-identical and checked. No speedup claim.
  Stub entry is not an equivalent baseline. Inherited diagnostic targets create
  no extra gate; mandated correctness, coverage and finite work bounds remain.
- Empty-PATH template source-to-ELF trace: 4800 specialization/body transitions,
  4809 native functions, 384567 text bytes. Source/object/mixed images also match.
- Required fixtures, references, bundle and comparison rules are unchanged.

| Increment | Handoff disposition |
|---|---|
| `b6eae734` | Initial ownership/scope plan; both review markers preserved |
| `ce08119c` | Coherent compile/object/link group plus 58 controls |
| `ff3e3848` | ELF load-address alignment, 61 controls and frozen performance runner |
| Evidence handoff | Final required validation and remaining implementation/review ledger |

Independent review remains pending: whole-stage source-to-ELF architecture,
object/alias/weak-definition ownership, runtime fact boundaries and performance
acceptance. Those review questions do not replace the unfinished implementation
above. The next independent audit must retain and resolve both categories.
Evidence: `/home/vishvananda/work/private/v4codex/artifacts/pa25-135/` and
[validation135](../student.tests/pa25/validation135.json). This handoff returns
control for further implementation/audit; advancement is not authorized by it.
