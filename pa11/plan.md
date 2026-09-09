# PA11 implementation

Stage base commit: a97e14d49c7edfc7acc115b974ab667cc90480db
Last reviewed commit: a97e14d49c7edfc7acc115b974ab667cc90480db

## Design and groups

Extend the integrated syntax/semantic graph and typed PA10 LowIR adapter. Stable
EntityId/TypeId own class layout, selected members and lifecycle demand; no text
roundtrips or fixture-specific behavior. PA12 value transfer and PA13 polymorphism
remain separate extensions.

| Group / owner | Data flow and complexity | Validation |
| --- | --- | --- |
| Layout and member calls / semantic class facts, lowering | Complete layout once per class; field offsets and method call types flow by IDs into address/call emission. O(members + demanded bodies). | layout, field, static/nonstatic method, inheritance fixtures; PA1–10 |
| Initialization and lifetime / semantic actions, lowering | Selected ctor and subobject actions; demand queue; intern lexical cleanup tails. O(actions + distinct cleanup states). | ctor/dtor, aggregates, arrays, global/TLS, goto and cleanup controls |
| Lookup, access, operators / semantic scopes and conversions | Indexed lexical/base/ADL edges, candidate conversions recorded once. Work proportional to required candidates/edges. | spec/access/friend/operator/overload fixtures |
| Bit-fields and alignment / layout and lvalue facts | Record storage, bit width/sign and align/union/volatile facts once; consume for loads/stores/initialization. | bit-field, alignas, packed and volatile fixtures |

## Performance evidence

PA11 is O0 source-to-LowIR. Preserve earlier benchmark evidence; inherited
self-selected diagnostic budgets are not stage gates (spec stage-scoped acceptance).
Measure frozen baseline/candidate latency and RSS on equivalent correct workloads,
A/A then ABBA with raw observations. Where LowIR can execute, measure runtime and
text size separately with the supplied backend. No optional optimization or runtime
profit claim without repeatable evidence; required semantic work is documented.

## Ledger / remaining work

- Entry: clean a97e14d4; prior turn produced authoritative baseline evidence
  (progress classification). PA11 43/302, 259 failures; PA1–10 and file audit pass.
- Inspection: class semantic facts exist, but lowering enumerates only namespace
  functions and has no field/member addressing. Implement foundational group first,
  then extend into related initialization/lifetime and lookup groups.
- All groups above remain open. Reference fixtures and comparison rules unchanged.
- Handoff: implementation active; no completion claim.
- Member-address foundation: selected calls record explicit/implicit object and cv
  ranking; emitted member ABI/signatures carry this; field projections consume
  cached layout, reference fields dereference storage, aggregate-array indexing
  uses byte offsets. Static members and direct unreachable terminators are wired.
  PA11 checkpoint 84/302 (218 failures); PA1–10 1025/1025, file audit pass.
  `student.tests/pa11/member-addresses.cpp` explicitly compiled with
  `--validate-lowir`, executed through supplied lowir2native-ref: exit 0.
  No performance benefit claimed yet; frozen stage baseline remains available.
