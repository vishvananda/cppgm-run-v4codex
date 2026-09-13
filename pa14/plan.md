# PA14 implementation plan

Stage base commit: `8af3c149454e4e43e441206e6978f4d1300e079b`.
Last reviewed commit: `8af3c149454e4e43e441206e6978f4d1300e079b`.
Target: **pa14 full-stage**. Phase: **implement**; architecture work remains.
Original entry **84/314**; continuation entry/current **314/314**.
All **230 original failures** are resolved. Coverage, references and comparison
rules are unchanged. PA15 has not started.

## Design/spec alignment

One parsed graph and canonical declaration/type/argument identities feed shared
fixed scalar, call, conversion and explicit receiver facts. Ordinary expressions
and type queries share member value/category rules. Source-owned receivers have
no temporary; concrete uses project object edges and establish required storage.
Repeated default expressions retain their semantic decisions while each emitted
materialization gets its own storage and cleanup address. Lifetime classifiers
follow semantic default edges and consume immutable conversion-call views.
Owners, standard rules and reducers are in [implementation.md](implementation.md).

| Remaining current-stage owner | Data flow, complexity and validation |
| --- | --- |
| Dependent body/object graph | Represent template-owned member/implicit-object declaration paths, substituted signatures/layout/access, constructor/operator expressions and declaration/return conversions. Replace whole-region projection with dependent-only facts. Key contextual facts by source plus canonical environment; follow required dependent edges. Validate access, unused-body legality, nested classes, lifetimes and scaling. |
| Demand/failure graph | Separate declaration/definition/layout/default/exception/body/vtable/emission states, typed reasons/reverse dependencies and structured expected failures. Follow demanded facts once per complete key; validate recursion, negative keys and unrelated-declaration scaling. Default evaluation storage is fixed; default semantic demand is still distinct unfinished work. |

**Concrete boundary:** the completed receiver cache accepts fixed class identities.
A template-owned receiver's member declaration, access path and layout depend on
its enclosing specialization. Publishing those in the source-only index would
mix environments. The next group needs symbolic declaration paths and complete
context keys across binding/substitution/member owners before it can remove
projection. Existing regions still project **77N/46N** nodes in the new receiver
corpora (50N/53N in the earlier call corpora). Extending the fixed cache alone
cannot safely implement that owner. These are current-stage requirements, not
waived by course success or performance acceptance. No external blocker exists.

## Performance evidence and budgets

The receiver/default campaign and retained follow-ups add **910 observations**,
**6,104 total verified**. All 35 common compiler/nine native outputs are identical;
three default-identity inputs have correct-B-only evidence, including a failing
entry executable. Final preflight preserves all these outputs. Compiler text is
1,253,062 bytes, **+11,072 (0.89%)** from entry; records remain 112/36/36 bytes.
Temporary cleanup records grow 28→32 bytes for the required concrete address.

At 4,000 fixed receivers, object-use records fall **20,003→8**, candidates
12,000→4,002 and RSS 1,612 KiB; compiler median .681021→.649173 s. Class-result
RSS falls 3,004 KiB, median .534766→.508689 s. Required checking of 4,000 unused
definitions adds 31.8 ms/3,076 KiB; eighteen reduced entry-accepted errors justify
the missing work. The final descriptor-view comparison has unchanged text and median RSS deltas -22 to +104 KiB,
modest ordinary-call benefit and mixed results elsewhere, including an unused
case regression. All outliers, CPU samples and earlier costs are retained in
[performance.md](performance.md); no general compiler/native speedup is claimed.

Work/storage follow source facts, semantic argument edges, concrete uses and
emitted materializations; lookups are indexed and completed effects are cached.
The fixed-fact reuse has a zero generated-code growth budget on common-correct
inputs, verified exactly. Correcting overlapping default objects requires storage
and cleanup; an invalid entry output cannot establish a profit comparison or a
zero-growth gate for that correction. O0 adds no optional optimizer/native
backend and has no mandated numeric compiler threshold. Historical self-imposed
gates remain diagnostics; mandated limits, correctness and coverage are preserved.

## Handoff ledger

Entry `d6891c36` is **verified progress** from the fixed-call/default-recipe group.
Earlier transfer/body/call commits and all 5,194 preceding observations remain
preserved in the linked reports and repository history.

| Coherent increment | Commit / evidence |
| --- | --- |
| Fixed receivers, explicit member calls, one member value/category owner | `80c344a6`; through 1935/1935, sixteen native controls |
| Frozen receiver scaling and broader object controls | `ba0490fc`; release/sanitizer parity on 330 inputs |
| Selected pointer completeness and callable-object routing | `ba6dcc2d`; twenty object rejections, including six pointer/query cases |
| Repeated default storage, cleanup identities and semantic effect edges | `607752d1`; seventeen native controls, branch/conversion/array reducers |
| Immutable conversion-call views | `70775cfd`; through 1935/1935, 331 sanitizer parity inputs, unchanged generated outputs |
| Frozen evidence/proofs | `f8eb4512`, `196c21f1`, `bb64e08d`, `4851afd2`; interrupted-harness samples and corrected follow-up retained |

Final checks: **314/314 PA14**, **1621/1621 earlier**, **1935/1935 through**;
seventeen native programs, two standalone reducers, **331** release/ASan/UBSan
status/output checks plus both reducers, **68** explicit sanitizer rejections,
two ABI controls and file audit (three inherited header advisories) pass.
Rejection parity and added controls are not counted as course progress.
Artifacts, exact commands/statuses, hashes, layouts and measurements are under
`$RALPH_ARTIFACT_DIR/pa14-object-facts/`. The receiver/default-use group is complete;
the two architectural owners above remain. Changes are committed and clean at
handoff; the full-stage goal remains active.
