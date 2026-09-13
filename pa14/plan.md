# PA14 implementation plan

Stage base commit: `8af3c149454e4e43e441206e6978f4d1300e079b`.
Last reviewed commit: `8af3c149454e4e43e441206e6978f4d1300e079b`.
Target: **pa14 full-stage**. Phase: **implement**; architecture remains open.
Original entry **84/314**; current **314/314**. All **230 original failures**
are resolved with unchanged coverage, references and comparisons. PA15 has not
started. Completed group: `97006205` → `dad19c39`.

## Design/spec alignment and ownership

| Owner / data flow | Complexity and validation |
| --- | --- |
| Checked callable signatures (completed) | Source declaration NodeId → retained Function TypeId and raw parameter types → canonical per-frame substitution → one concrete parameter publication. Prototype query entities keep ordinals/access contexts without replacing source/body identity. Controls execute cv/array/function parameters, trailing returns, nested declarators, overloads and cleanup. |
| Complete-class source uses (completed) | Typed default/initializer uses queue once at the owning source class and drain after enclosing declarations complete. Separate queued/active/complete/failed states prevent duplicate work; detached local batches permit reentrant completion and release in bulk. Later private/nested lookup, initializer order and overridden dependent defaults/initializers execute correctly. Static initializers retain declaration-point lookup. |
| Enum identities (completed) | Member/local/anonymous enum declarations retain Named identity; current-instantiation signatures canonicalize to it. Concrete enum publication supplies the per-frame binding before queries/signatures consume it. Renamed/nested heads, enum widths and overload separation pass. |
| Inherited owners (completed) | Source/local declaration bindings, sparse stable Facts, expression/value queries, immutable regions, defaults and selected definitions retain their previous evidence. No syntax replay or source-fact recovery through projected occurrence IDs. |
| Remaining type/query graph | Some embedded dependent type/query forms still require declarator construction. Joint source identity, scope, object/lifetime and query producers must exist before removing their concrete semantic work. Whole-region occurrence IDs and optional Fact indices remain. |
| Remaining demand/failure graph | Integrate independent declaration/definition/layout/default/exception/body/vtable/emission states with typed reasons, precise reverse dependencies and structured expected failure. Current source-completion uses cover defaults/initializers; they do not replace these distinct concrete owners. |

**Concrete boundary:** this continuation completed the joint signature/raw-parameter
change, then extended it through exposed late-default and initializer failures
and enum identities. Remaining work crosses `template_definition`,
`template_instantiation`, `default_arguments`, layout, exception and emission
owners. A source completion queue cannot safely serve as their shared concrete
fact scheduler: source definitions can arrive later, recursive declarations may
be usable before definitions finish, and defaults/bodies must remain independent.
That requires a separate producer/consumer and failure-key audit with insertion,
recursion and demand scaling controls. There is no external blocker.

## Performance evidence and budgets

**13,776 observations**: 12,838 inherited, 924 from the full 54-input/12-native
campaign and fourteen from an unchanged wide-case repeat. All 54 LowIR and twelve
native hashes match exactly. New N/K/Q signature cases improve median latency
**5.87 / 5.72 / 4.51 / 0.68%**, each in both ABBA blocks; peak RSS falls
**10,632 / 63,294 / 29,430 / 11,818 KiB**. Source signature work is K+1,
applications N(K+1), raw parameter publications 6NK, and source default/initializer
work each K, independent of Q. Additional keyed substitution/canonicalization
work remains disclosed. Generated growth is zero; no runtime gain is claimed.

The inherited wide local case initially showed +13.18% latency during an
unisolated eleven-second → 25–31-second transition affecting both binaries.
The unchanged repeat returns to eleven seconds and B improves **0.84%** in both
blocks. Both observations remain intact. Repeated-special has **+17,564 KiB RSS**
despite fewer Facts, with allocation cause unisolated, while latency improves
2.03%. Other inherited increases and outliers remain in performance.md. Correct
late-member proofs use `7ef73440`; rejected entry sources are never timing baselines.

Current compiler text: **1,317,638 → 1,320,774** (+3,136 bytes, 0.238%).
Analyzer: **6000 → 6152** bytes; typed class use: **20**. Other public hot-record
sizes remain unchanged. The current probe owns all 18 live transitive headers;
older probes retain snapshot integrity. Historical layout/measurement data and
RSS costs remain in [performance.md](performance.md).

Explicit budgets: source/key/concrete-use-proportional storage, at most four O0
conversion variants per source operation, one local writable view per Fact
publication, and zero generated growth. PA14/O0 mandates no numerical
latency/RSS/compiler-text ceiling. Historical diagnostic equations/layouts are
snapshot evidence, not permanent implementation gates. No mandated limit,
correctness requirement or coverage was removed.

## Handoff ledger

| Increment | Commit / evidence |
| --- | --- |
| Inherited owners and performance | Through `97006205`; 12,838 observations and all earlier proofs retained |
| Retained signatures/raw parameters and complete-class defaults | `84567172`; original missing-parameter failure and identity traces preserved |
| Typed complete-class member initializer uses | `7ef73440`; valid late initializer entry rejection and native order/override controls |
| Direct member enum identity | `dad19c39`; correct qualified intermediate also preserved |
| Proof/sanitizer/required validation | `85d3acd6`; six native proofs, 118 recorded checks, all required reports pass |
| Performance acceptance | `signature-publication-handoff.json`; 938 new observations, all 13,776 cumulative observations verified |

Validation: **314 stage / 1621 prior / 1935 through**, **33 native programs**,
**347 release/ASan/UBSan parity sources**, **166 required rejections**, one optional
unused-default diagnostic, six ABI controls, seven inherited and four new native
reducers, plus initializer/store/lifetime checks. File audit passes with three
inherited header advisories. All **1266 fixture/reference files** remain intact.
The first proof's unsupported mandatory unused-default diagnostic is preserved;
N3485 [temp.decls]/2, [temp.res]/8, [temp.inst]/1,12–13 justify the corrected
optional observation plus a separate demanded-use rejection. Artifacts:
`$RALPH_ARTIFACT_DIR/pa14-signature-publications/`.
