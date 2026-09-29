# PA20 implementation plan — handoff 94

Stage base commit: `a9b24ab68f1a75288df10161cb171fa239e1409a`.
Last reviewed commit: `a9b24ab68f1a75288df10161cb171fa239e1409a`.
Target: **PA20 full-stage**. Phase: **implementation handoff; stage incomplete**.
Entry: **73/144 pass, 71 failures**; PA1–19 and file audit pass.

## Design and remaining groups

| Owner | Data flow / complexity | Validation |
|---|---|---|
| Placeholder deduction (implemented) | Source declarator + checked initializer/returns → canonical type → recorded conversions; function/specialization body state owns recursion/failure, cached type predicate, O(returns + type shape) work | Ordinary/member/template/lambda returns, pointer/reference/cv/collapse, addresses, constexpr obligations, recursive/conflicting returns, condition declarations |
| Expression initialization / conversion (implemented) | Clauses → completed canonical array type and typed list plan; one constructor selection before aggregate fallback; selected modifiable-reference conversion + computation type → direct LowIR | Unknown-bound/nested/pack/string array expressions, unevaluated queries, aggregate construction, alias conversion using-declarations, prefix/postfix including volatile/pointer results |
| Aggregate helpers / initialization shape (partly implemented) | Scalar helper key = target + explicit prefix; transfer helper key = full semantic plan. O(fields), at most fields+1 scalar shapes per target; existing array expansion budget remains 8 | Distinct omitted prefixes pass; remaining nested-array/member-transfer/constant-image LowIR contracts need separate emission work |
| Range statements (unfinished) | Range fact owns single evaluation, begin/end selection, iterator operations, iteration variable and lifetimes; typed operations must feed lowering without fake syntax | Arrays/lists/member/ADL/inherited ranges; references, prvalue lifetime, template composition |
| Closures (unfinished) | Source occurrence + enclosing specialization → closure identity, captures/call/conversion/ABI facts → helper emission | Captures including nested this/local, return deduction, defaults, special members, local statics, helper shape |
| Retained declarations / lifecycle (unfinished) | Complete enclosing environment → declaration/probe fact → local lifecycle validation; reuse by complete context | Dependent-owner lifecycle and repeated local declaration probes |

Extended the deduction owner through array/aggregate operand construction and
selected lvalue conversions. Existing typed graph,
canonical IDs, scoped fact caches and direct LowIR remain the production path.
No references, fixtures, statuses or comparison rules changed. All 637 tracked
contract/harness files match stage base, including all 144 stage inputs.

## Performance and handoff ledger

Frozen entry binary `/tmp/pa20-entry-cppgm` (SHA256
`1a181c95ee97d646d41c1009780200049e776d67e1d715840d5526b3d10e1d40`).
[Performance and source-to-native evidence](performance94.md) retains all A/A
and ABBA observations, compiler latency/RSS and checked runtime/payload sizes.
Final compiler SHA256 `29cfe4f4ced25eabdc3d652a91b275c74455f855a090029d3497207ed88bf5bc`;
text +6,336 bytes (0.316%). Eight comparable native files are byte-identical.
Final 9,600-specialization compiler medians 632/664 ms, peak RSS 107,016/106,012
KiB; noisy paired ratios 0.926–1.060. Body checks remain n+1, deductions n,
placeholder-type work 2. No general speedup is claimed. Required semantic costs
are bounded; no optional optimizer is added. Inherited percentage diagnostics
are not extra PA20 gates under spec §9; mandated limits remain in force.

`dbd1a8f6` implements placeholder return facts, including shared lambda returns.
`df239d8e` extends typed array/list construction, functional aggregate fallback,
selected increment/decrement conversions, using lookup and scalar helper keys.
`0dd795a1` removes scratch substitution tables for bare value placeholders.
This evidence commit records the validated handoff and unchanged coverage.
[Required checks](../student.tests/pa20/validation94.json): personal controls
**83/83** (59 native, 24 required rejections), plus a native architecture trace.
`make test-pa20`: **91/144**, exit 2; through report:
**3543/3596**, all **3452 earlier tests pass**, PA20 **91/144** (53 failures,
18 original failures fixed, no new failures). File audit passes with three
inherited header warnings. Stage-progress criterion passes; the full stage does not.

Remaining failures by owner: **17 ranges, 29 closures, 5 initialization output
contracts, 2 retained-declaration/lifecycle cases**. Exact names and the original
failure-set comparison are in the validation ledger; these are implementation work.

Handoff boundary: the completed type/conversion owners have native and rejection
controls. Range-for has no semantic/lowering owner yet; captures require new
closure environment/lifetime facts; remaining initialization shape requires
separate aggregate transfer/array image emission decisions. Broadening scalar
helpers into value initialization caused an earlier-PA regression during work;
that extension was removed and the earlier suite revalidated. More shortcuts
in deduction/conversion cannot supply these missing owners and their lifetime/
output contracts. Full through-PA20 remains required before stage advancement.

Independent review remains pending for all three implementation commits, including
placeholder demand in unevaluated contexts, query/source parity, helper keys and
performance evidence. The preserved review marker does not waive any finding.
