# Implementation193 — effective member ABI attributes

Entry `71b9f44aa662154c0c072fd9e4df6510324fce09`: PA29 **402/403**.
Owner: attribute selection for retained class-template member definitions,
including canonical identity across instantiation and explicit specialization.
This completes the previously unresolved owner; independent stage audit remains.

## Contract decision

The unchanged course fixture
`tests/run/800-out-of-class-nested-template-abi-tag-suppression-run` requires an
untagged definition and forbids the declaration tag in defined symbols. Its
source, inspection expectations, reference output, status and comparison rules
are preserved. This is a hosted extension compatibility rule, not a claim that
C++11 requires Clang's behavior or that GCC is wrong.

[GNU's documentation](https://gcc.gnu.org/onlinedocs/gcc/C_002b_002b-Attributes.html)
describes retention on template instantiations; [Clang's ABI-tag description](https://clang.llvm.org/docs/ItaniumMangleAbiTags.html)
describes encoding but does not settle this definition-selection discrepancy.
The course contract is authoritative for this supported hosted surface. No
reference correction or bundle change is made. The old review question in
[implementation176](implementation176.md) is resolved by implementing that
contract as a general semantic policy, not a name or source-pattern exception.

The [entry matrix](../student.tests/pa29/evidence193/entry-probe.json) varies ordinary,
direct, nested, deeply nested and member-template class owners with inline,
out-of-class, repeated-tag and declaration-only functions. The distinction is
**which declaration supplies attributes when a dependent member is instantiated**:

- A matched out-of-class definition already visible at member instantiation
  supplies its explicit ABI tags, including an empty set.
- Without that definition, the member retains its declaration tags. Instantiating
  the class establishes that choice even before any member call. Later definitions
  do not rename the member; addresses, calls and definitions stay consistent.
- Function-template instances inherit their member pattern's effective set.
  Ordinary non-template members, free templates, inline bodies, tagged enclosing
  types and static data keep their existing attribute behavior.
- An explicitly specialized non-template member supplies its own attributes;
  specializing a whole class still uses its ordinary member declarations.

Clang observations support the generalization; they are supplemental controls,
not sufficient proof to revise an oracle. The matrix includes arbitrary names,
different tag sets, overloads and source order. No compiler is invoked by the
implementation to produce output.

## Ownership, data flow and complexity

Parsing retains attributes on immutable source nodes. The existing semantic
signature index matches an out-of-class definition to one canonical member
prototype. `check_template_member_definition` publishes that definition's tag
head in a TU-owned flat index keyed by prototype identity. A nonzero encoded
head distinguishes a known empty set from no matched definition. The source
member's declared attribute list remains intact.

Concrete declaration publication already records its prototype in `MemberFacts`.
It now snapshots the selected effective tag head in a second flat numeric index,
keyed by concrete entity. This is also the boundary for class instantiation before
a definition. No global retry, invalidation, lexical scan or string-keyed lookup
is added. A function specialization inherits the effective set through the
existing canonical pattern edge. Lowering calls `effective_abi_tag_head` and
builds the existing typed `Tagged` name. The ABI graph emits the same name for
calls, function addresses, definitions and ELF relocations. Serialized LowIR is
an explicit adapter, not production phase transport.

Each selected source prototype and concrete member publishes once. Lookup is
O(1) average; tag copying is O(actual tags) at specialization, as before. Storage
is O(selected prototypes + instantiated members + existing tag records), in
geometrically grown TU indexes. Source and instance tag lists share retained
heads where possible. All facts release with the analyzer; no process-global
cache or new per-node owning allocation is introduced. No implementation source
was added, so source-set registration is unchanged.

The related explicit-specialization guard formerly demanded that the immediate
class itself have template arguments. It now uses the existing cached
`definition_owner` fact, which also handles non-template nested classes of class
templates and stops at an explicitly specialized owner. Signature and static-member
checks still apply. Selected member specializations reset only their own inherited
attributes; primary declarations and other instances retain theirs.

A negative control found another existing ordering defect: a queued, used
function could be explicitly specialized before its deferred body ran. Selection
now checks the existing use fact as well as completed/in-progress body facts.
N3485 [temp.expl.spec]/6 (local `doc/n3485.txt`, line 20011) requires specialization
before a use causing implicit instantiation. This program is ill-formed without
a required diagnostic; the compiler now rejects it consistently. Class completion
and unevaluated queries alone still allow subsequent member specialization.

## Validation and handoff boundary

The explicit harness `student.tests/pa29/check193.py` covers O0/O2 symbols and
execution, instantiation order, overload isolation, const member pointers,
member-function templates, tags on enclosing classes, tag sorting/deduplication,
static data, ordinary/free templates, nested explicit specialization and invalid
specialization boundaries. A Clang-generated peer instantiates an extern-template
member and resolves the compiler's undefined symbol. Five programs also cross
LowIR read/write/native adaptation; ELF symbols, relocations, unwind frames and
MIR are inspected. Telemetry-on/off objects must match.

Final counts, coverage, source binding and performance are recorded in the compact
[plan](plan.md) and [performance report](performance193.md). The first personal
run exposed the late-specialization defect; its evidence is retained rather than
silently replaced. Required course checks and the explicit harness are rerun on
the final implementation. No known unfinished behavior remains in this owner.
Whole-stage architecture/correctness/performance audit, including the unreviewed
implementation191–193 range, is a separate required boundary before advancement.
