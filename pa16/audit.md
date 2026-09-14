# PA16 accumulated checkpoint audit — loop 40

Stage base commit: `438d56b164600f4fa19d25dcb5f09a76e2a79776`
Last reviewed commit: `7c39a6edbfa43c226036b8a92fe236722ac85dcc`

Reviewed entry: `57bac58d`, clean, **93/154 with 61 failures**.
This first PA16 review uses the stage base, as both original markers required.
The complete range includes twelve original commits, the repair commit and the evidence-verifier correction.
The [manifest](../student.tests/pa16/audit-checkpoint.json) enumerates all fourteen
commits and hashes all 39 changed implementation paths. The combined diff and
handoff interactions were reviewed. This checkpoint audit does not certify
completion of the unfinished full-stage features in [plan.md](plan.md).

## Accumulated range

| Commits, in source order | Review disposition |
|---|---|
| `d7594d63`, `482c6b44`, `7484f22b`, `cc842756` | Scalar frames, floating payloads, array projections, tag/ordinary lookup ordering, empty value initialization and scalar evidence. Includes the intermediate tag repair and conversion/cache boundaries. |
| `4aab464e`, `66f123ab`, `e85d39a9`, `c2ccf2d5` | Semantic static classification, persistent locals, guards/destructor callbacks, string references, structural data interning and counters. Includes zero ranges, separate automatic identities and storage campaigns. |
| `65c09c3b`, `3ff1c1db`, `3a836964`, `57bac58d` | Inherited plan, validity/cv, deferred exception specifications, expression/initializer/allocation/destruction effects, stateless receivers and evidence. Includes completion intervals, redeclarations, defaults, specialization obligations and body demand. |
| `83536144`, `7c39a6ed` | Repairs below, explicit audit controls/evidence tools and historical snapshot verification; resulting combined source re-reviewed. |

Documentation handoffs preserve measurements and unfinished work; they do not
establish semantic completion. No contract fixture, reference, bundle, comparison
rule or attribution changed in the reviewed range.

## Findings and repairs

1. **Two contexts for one specialization.** A dependent exception specification
   created a private declaration context; later body instantiation attached its
   canonical substitution frame to another context and failed an invariant.
   Reverse demand order also failed. Reducer:
   `template<class T>constexpr int f()noexcept(sizeof(T)>1){return 7;}`, followed
   by `static_assert(noexcept(f<int>()),"");` and
   `static_assert(f<int>()==7,"");`. The exception owner now reuses/publishes
   `Specialization::context`, as defaults and bodies do. Demand states stay
   separate; exception demand still requests no body. This repairs spec.md
   §§2/4/5 identity and once-per-fact requirements and the README's call/noexcept
   behavior. Controls cover both orders, defaults and multiple specializations.

2. **Unavailable construction cached as permanent rejection.** A namespace
   initializer called a scalar constexpr wrapper before its empty constexpr
   constructor was defined. Receiver/activation caches retained failure, rejecting
   a later valid assertion. Reducer:
   `struct A{constexpr A();constexpr explicit operator bool()const{return true;}};`,
   `constexpr bool f(){return bool(A());}`, `bool early=f();`,
   then `constexpr A::A(){}` and `static_assert(f(),"");`.
   The receiver owner now propagates unavailable status and withholds negative
   publication until the definition prerequisite is complete. Completed successes
   and genuine failures retain memoization; no global cache clear/retry was added.
   [N3485](../doc/n3485.txt) §5.19 [expr.const]/2 excludes an undefined constructor
   invocation, not a later invocation after definition. §3.6.2 [basic.start.init]
   and the README allow ordinary initialization to fall back dynamically.
   Ordinary/template wrappers now execute correctly; missing/effectful constructors
   remain rejected in required constant contexts.

3. **Initializer conversion omitted from constexpr validation.**
   `constexpr int x=1.0e30;` and `constexpr float x=1.0e300;` were accepted
   because validation preceded conversion. The declaration owner now validates
   the converted value before publication. [N3485](../doc/n3485.txt)
   §7.1.5 [dcl.constexpr]/9 includes implicit initializer conversions;
   §§4.8 [conv.double]/1 and 4.9 [conv.fpint]/1 make out-of-range conversion
   undefined, excluded by §5.19 [expr.const]/2. Controls cover implicit float/int
   overflow and negative floating-to-unsigned conversion.

**Investigated and retained:** a proposed single-rounding double policy was
withdrawn. For `b=2^-53+2^-66`, x87 evaluation of `1.0+b` followed by double
materialization may produce 1.0. [N3485](../doc/n3485.txt) §5 [expr]/12 permits
excess precision; supplied-backend disassembly uses `faddp` then `fstpl`.
The original floating engine matches that model. The arithmetic trial introduced
a native mismatch and was removed before the reviewed commit. Original reducers,
trial results and disassembly bytes remain in the manifest; final controls check
permitted values and native parity. No reference correction is warranted.

## Architecture and optimization traces

The `source_to_native` audit control combines demanded `step<double>`, noexcept,
two equal constexpr double arrays and a dynamic local static. Immutable buffers
feed streaming interned tokens and integrated parser/analyzer construction in
`lowering/driver.cpp`. The body is parsed once; `syntax/occurrence.cpp` projects
compact occurrences into the specialization context, with parent-linked
substitution frames and shared fixed facts. Selected declarations/conversions,
completed exceptions and object/initializer identities feed direct typed LowIR.

Telemetry records **one** template body transition, two scalar activations,
three completed-call hits, twelve execution steps, one local-static owner and
one readonly data record. Lowering emits **68 instructions**, two distinct stack
objects and two `copyobj 16x8` operations from the same immutable image.
The guard encloses the real `seed()` call and changes after initialization.
Native execution checks values, distinct automatic identities and one-time
initialization. `step<double>` carries semantic `unwind=no` into its signature;
unknown effects remain conservative. At 4,000 specs, declaration-only demand
checks zero bodies; spec/body pairs compute exactly 4,000 bodies/activations.

Additional controls cover precision, signed/unsigned overflow, volatile reads,
scalar mutation, omitted ranges, tag lookup, deferred/redeclared specs, conversions,
defaults, allocation versus temporary destruction, literal/class completion,
persistent references and destructor order. Identity uses TypeId/EntityId/QueryId
and typed plans, never rendered text. Exception traversal follows actual operand,
conversion, initializer and lifetime edges. Completion drains its queue interval.
Scalar activation keys cover the body, converted arguments and supported stateless
receiver; general object/alias/lifetime inputs are explicitly unfinished.

Mutable frames use call-local flat storage released on return. Constants, floating
payloads, completed facts and array indexes live in TU vectors/flat indexes.
No new hot per-node owning allocation, duplicate syntax tree or process-global
mutable cache was introduced. Frontend state releases after each TU's lowering;
function builder scratch releases after each function; the required LowIR program
releases after its output writer. No textual representation transports internal
production phases.

The useful optimization fact is a fully constant, nonvolatile initializer image.
Legality follows its semantic plan and immutable payload; separate stack storage
preserves automatic-object identity. Interning compares item kind/type/value,
symbol/addend, size and alignment, excluding x87 padding. A duplicate tentative
data tail is released. Work follows visited/emitted items; omitted ranges remain
compact, and array lookup searches explicit child ranges logarithmically.
The inherited eight-element expansion cap retains a loop fallback. Array copying
is a PA16 requirement; its runtime benefit and code/data tradeoff are measured in
[audit-performance.md](audit-performance.md). There is no optional optimizer,
fixed-point pass, inlining/growth policy or unbounded profitability search.

PA16 produces typed LowIR and its requested text adapter. The harness explicitly
validates/reparses external LowIR and invokes the supplied backend to obtain ELF.
That authorized validation boundary implements none of the compiler's frontend
or lowering output. Own MIR/allocation/encoding/object writing and optimized
debug/self-host acceptance belong to later PAs; PA16 requires no extra debug target.

## Validation, remaining work and ledger

Fresh final-source checks: `make test-pa16` **93/154**, exit 2; exact requested
prior-through command **2112/2112**, exit 0; through PA16 **2205/2266**, exit 2
only for the same 61 PA16 failures; file audit passes with three inherited header
warnings. Root reports ran sequentially. All **148 native / 66 rejection** personal
controls pass, including 22 new controls. The [verifier](../student.tests/pa16/verify_audit.py)
checks failure identities/count, unchanged contract trees, source/binary hashes,
markers, current campaigns and historical evidence.

Remaining work stays broadly grouped in the plan: general object/address/lifetime
evaluation plus initializer obligations; publication/storage/emission; ordinary
array copy/oracle conflict. Structural constructor validity does not certify
unfinished initializer contents. All 61 failures, including invalid acceptances,
remain explicit; additional personal passes compensate for none of them.
Store and copy alternatives both preserve C++ behavior, so the array conflict
does not satisfy the wrong-reference exception. The README obligation is unwaived.

The scalar/storage/validity handoffs fragmented connected ownership work and
missed the exception-to-body control. Complete the next object/initializer/lifetime
group cohesively and exercise demand orders through storage/emission.

| Audit | Reviewed range and disposition | Evidence / remaining work |
|---|---|---|
| Loop 40 checkpoint | `438d56b1..7c39a6ed`; all 3 handoffs reviewed; 3 defects repaired; code tip frozen before records-only commit | 93/154, same 61 failures; prior 2112/2112; file audit; 854 new + 2,658 preserved observations. Checkpoint accepted; full-stage work remains in the three plan groups. |
