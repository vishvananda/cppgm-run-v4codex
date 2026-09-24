# PA18 implementation handoff 67

Entry `06211ad0438df250952a414eef409d657b0ff5b0` → implementation
`0b60ca52` (with `626b5809`, `d5497ec7`, `f2cd086f`). PA18 **327/420 → 343/420**:
**16 original failures fixed, zero regressions, all 420 fixtures retained**.
Earlier PAs pass **2609/2609**. File audit passes with the same three inherited
header-division advisories. PA18 full-stage completion remains unproved:
**53 status failures and 24 LowIR mismatches** are required unfinished work.
This is an implementation boundary, not an independent audit or advancement.

## Completed behavior and ownership

| Owner | Data flow and obligations | Work / lifetime |
|---|---|---|
| `template_arguments`, `template_address_arguments` | Retain typed lvalues/overload queries until the NTTP target is known; apply C++11 qualification, direct reference binding, array/function decay, null and overload selection rules. Check expression form, linkage, storage duration and forbidden subobjects. Canonical values retain constant storage identity, never a rendered symbol. | Required expression/overload candidates only; completed conversions cache query, target and access overrides. Incomplete/failed results retain existing prerequisite owners, not an unsafe negative cache. Local scratch releases on return; query/address facts release with TU. |
| `template_entities`, `template_class` | Accept pointer/reference/function/array parameter declarators, packs including nested declarators, and reference value categories. Defaults and parameter types consume the immutable outer lexical frame plus earlier bindings. | Traverse the declaration/head once; cached lexical frames, identity-keyed prefix overlays. No token replay or copying of the full enclosing environment. |
| `type_query`, constant/query consumers | Concrete implicit address substitution publishes one canonical argument result. Reference arguments preserve lvalue and declared-reference facts. Member naming/access context belongs to the query; access checks have a nonthrowing predicate for candidate failure and diagnostic wrappers for hard errors. | Cached by query and immutable substitution frame. No global cache clear, retry pass or exceptions for ordinary address-candidate rejection. |
| Demand and ordinary lowering | Address arguments queue required function definitions, including hard errors from bodies during candidate checking; ordinary address/reference uses demand static storage. LowIR consumes constants, object identities and selected declarations. | Existing deduplicated demand owners and typed IR; no new optimizer, semantic recovery, host delegation or text phase transport. |
| Typed ABI and linkage | Distinguish pointer-address and reference-entity encodings. Structured external literals use the enclosing substitution dictionary. Internal referents propagate through arguments, packs and specialization owners before symbol merging. | Canonical graph IDs; TU-local dense linkage caches compute completed facts once. Output symbol state owns its substitution table. |

N3485 14.3.2 [temp.arg.nontype]/1,5 governs the address conversions and restrictions;
3.2 [basic.def.odr] and 14.7.1 [temp.inst] govern required function definition
uses. Type argument identity remains consistent with 14.4 [temp.type].
The exact ABI contract and one corrected historical oracle are documented in
[reference-correction67.md](reference-correction67.md). All PA18 sources/oracles,
all success/failure statuses, all comparison rules and every fixture remain.
Only that proved PA9 `.ref` differs from entry.

The initial address group was extended through explicit function template-ids,
target deduction, pointer/reference packs, outer parameter-type/default frames,
static storage demand, ABI substitution and cross-TU internal linkage. These
were directly related defects, not deferred review questions. No known defect
in this completed group is being waived.

## Validation

- `make test-pa18`: **343/420**, exit 2; stage-progress reduction **93 → 77**.
- Required `n=18; … make test-report-through-pa$((n - 1))`: **2609/2609**, exit 0.
- `perl scripts/cppgm_file_audit.pl --stage pa18 --paths dev/src`: exit 0.
- Explicit personal address controls: **63/63** (40 executable controls and
  23 rejection controls); fixed ABI controls **13/13**; two-source linkage
  executions **5/5**. They test identity and observable effects, not diagnostics.
- Repeated-address controls **3/3**: 32, 128 and 512 uses each inspect exactly
  three candidates in total, validate LowIR and execute with the checked result.
- Preserved ordering **64/64**, substitution **33/33**, conversion **51/51**,
  and independent-audit cache controls **5/5**.
- All **16** repaired course fixtures additionally validated; **14** compare
  executed outcomes against their preserved reference LowIR, and two have no
  entry point and are validated as LowIR only.
- [Performance evidence](performance67.md) records the final frozen A/B protocol,
  all observations, compiler RSS/latency and checked executable runtime/size.
  The earlier run is retained separately and is not the final ABI acceptance.
- [Evidence ledger](../student.tests/pa18/loop67-evidence.json) records hashes,
  check results, complete fixed/remaining failure sets and control outcomes.

## Handoff boundary and independent review

The remaining failures require other owners: retained member/alias and correlated
pack contexts; dependent expression validity (compound assignments, braced lists,
casts, destructors); constructor/inherited constructor participation and explicit
ADL deduction; and LowIR array initialization/result/ABI metadata. These are
unfinished implementation, not advisory audit questions. They need their own
expression/type/constructor or initialization facts and cannot be repaired by
further widening address conversions without violating C++11's restrictions.
Their full-stage scope remains in the compact plan.

Independent review is still required for the whole accumulated stage range,
including this handoff's address-argument definition-demand boundary, canonical
reference/pack identity, immutable outer-head defaults, internal-linkage cache
keys and the ABI oracle proof. These are review topics for implemented behavior,
not a claim that audit has passed. Stage base and last-reviewed markers are
preserved. Do not advance to PA19 until the root through-PA18 report passes and
Ralph resolves the independent whole-stage findings.
