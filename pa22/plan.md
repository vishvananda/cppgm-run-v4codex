# PA22 compact plan — handoff 119

Target: **PA22 full-stage**. Phase: **validated implementation; independent audit pending**.
Stage base commit: `a8482d768bd2dcede42ea63ef39e39cf3245c380`.
Last reviewed commit: `e90fa3fa514e2990bbe5ea4716252d8c42ae782a`.
Entry HEAD: `17603c8a69e76820274b7bf849091d528f6ff261`, **95/99**, clean.
Implementation: `321c93db`, **99/99**. Previous goal turn: progress (handoff 118);
no live build/test process remained at entry. Review markers are unchanged.

## Design/spec alignment and completed groups

- **Parameter member values** retain generic signed receiver adjustment and
  target-word truth. An external parameter has no zero-adjustment guarantee.
  Four minimal [oracle corrections](reference-corrections119.md) have reduced
  reproducers, C++11/LowIR proofs and original/corrected hashes with bundle
  revision. All 99 sources, status sidecars and comparison rules are preserved.
  The fourth correction removes an undemanded static template member; its
  address-taking control still requires storage. The narrow-integer LowIR
  truth compatibility rule now rejects implicit truncation from `i128`.
- **Constant class receiver materialization** is owned by each selected user
  conversion. A completed, receiver-independent scalar body summary feeds a
  bounded proof of fresh empty construction plus inert destruction. Canonical
  function IDs index source/conversion-use edges; typed lowering consumes a
  boolean fact. No syntax replay, callee demand, text keys, tree copying or
  additional whole-program scan. Effects, named objects, nonempty classes,
  list initialization and unknown facts retain ordinary evaluation.

Receiver proof work is at most **8 nodes/use**, with one use edge per prepared
scalar conversion and cached existing constructor/destructor action facts.
Result-body proof remains bounded by **8 wrappers**. TU-owned contiguous pools
and flat indexes release with the TU; only the use fact is consumed by lowering.
There is **zero generated-code growth**, no duplication and no fixed-point pass.
Inherited limits remain: 64-node member-value proof, 4096 flow visits/function,
64 flow depth, and eight-element initializer expansion before loop fallback.

## Validation and performance

[Validation](../student.tests/pa22/validation119.json): `make test-pa22` **99/99**,
prior through PA21 **3712/3712**, full through PA22 **3811/3811**, file audit pass
(three inherited header warnings), **126/126** personal controls plus ABI truth
and reference reducers, **95** stable LowIR roundtrips and **four** required
rejections. The original entry passes all 28 initial behavioral controls but
fails 11 receiver-elision expectations. The final 32 new controls cover effects,
lifetime order, depth fallback, real static-member demand and thrown exceptions.
The supplied standalone backend's pre-existing fundamental-RTTI limitation is
retained in evidence; the two exception controls execute through the unchanged
supplied object backend and host runtime, as documented in PA21.

[Performance](performance119.md) records frozen A/A+ABBA compiler latency/RSS,
checked runtime/text size, all observations and the scalar-loop regression
investigation. Receiver visits scale **512→2048** with exactly that many uses.
Six common workloads retain identical IR/text. Compiler text grows **1920 bytes**.
Spec §9's PA22/O0 acceptance applies. Historical +15%, +16 MiB and 5.5× diagnostic
targets remain non-gating; all 114–118 measurements and mandated work/growth,
correctness, comparison and coverage constraints are preserved.

## Handoff boundary and independent review ledger

**No known PA22 implementation failures remain.** The two entry ownership groups
are complete, including receiver effects, typed truth validation and proof-backed
reference repairs. PA23 owns virtual inheritance, polymorphic multiple inheritance
and broader RTTI. Do not advance until the whole-stage independent audit resolves
its findings; passing this handoff does not waive or certify that audit.

| Range | Status | Evidence / outstanding review |
|---|---|---|
| Stage base through `e90fa3fa` (114–117) | Independently reviewed | [audit117](audit.md), including both audit fixes; historical evidence preserved |
| `f18dfb62..e10bdd7f` (118) | Implemented; independent review pending | Local member storage, qualified repeated-base paths and overloaded-arrow effects |
| `247c7de4..321c93db` (119) | Implemented; independent review pending | Constant receiver use/lifetime ownership, wide truth validation, four documented oracle corrections |

Independent review must cover accumulated whole-stage architecture, occurrence
identity and conservative effect boundaries, proof budgets/profitability, native
placement sensitivity and the oracle proofs. Those review questions are distinct
from unfinished implementation; neither category is waived. The implementation
handoff returns control to Ralph for that audit.
