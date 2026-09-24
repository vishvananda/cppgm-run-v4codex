# PA18 checkpoint audit 66

Stage base commit: `94dcb8ad21664137e87d574e878c14a4a047348a`.
Previous Last reviewed commit: `94dcb8ad21664137e87d574e878c14a4a047348a`.
Last reviewed commit: `3a883d10a27e41d1b126eaef05eaf0b454de1646`.
Entry: `0619704c7ab0323e35a282c93bf8c0630429770d`.

**Checkpoint audit passes; PA18 implementation remains incomplete.** All 15
accumulated commits and their combined changes were reviewed, followed by the
audit fix commit. This is the first review, so the boundary is the stage base,
not the latest handoff. There are 47 changed implementation/registration files.
[Machine-readable evidence](../student.tests/pa18/loop66-evidence.json) records
all full commit IDs, source and fixture hashes, controls, trace and check results.

## Range and interactions

| Commits (chronological within each group) | Review |
|---|---|
| `bddf7d2d` | Stage boundary and original ownership plan; recovered and checked against history. |
| `5d71a342`, `e84a420e`, `c576e6a5`, `4a49ea10` | Call/address partial ordering; cv/reference/pack rules; winner verification; prototype result/pack lanes; direct sizeof-pack ABI. Reviewed the initial implementation and subsequent lane/cached-probe corrections together. Cache lookup now precedes shape construction; runtime object lanes do not index the type arena. |
| `1863f18d`, `30a61006` | Frozen observations, executable controls and handoff 63. Historical failed scaling attempt and pre-cache measurements are retained. |
| `d15a0328`, `e01b8763` | Explicit and deduced immediate substitution; structured query/array failure; completion dependencies and failure caching; direct increment/decrement ABI. Checked candidate failure versus hard class/body side effects and integration with ordering and prototype queries. Finding A below repairs the failure-cache lifecycle. |
| `3f49ce3a`, `52d97223` | Correct the personal execution oracle to the fixture's actual return value 2; preserve the course contract; handoff 64 and performance records. |
| `5d8a136e`, `644dd88b` | Target-driven conversion deduction, exact result conversion, return-only ordering, explicit calls, hiding identity and member-head prefix substitution; constant reference address consumption; parser continuation and template-head mode restoration; original conversion-type ABI. Reviewed in combination with the earlier ordering and dependency owners. |
| `4eaf273d`, `0619704c` | Proved reference correction, reducer and handoff 65. Only one `.ref` changed; proof reviewed independently below. |
| `3a883d10` | Fix A and B, reduced controls, and the audit benchmark harness. All implementation fixes validated and committed before this record/plan commit. |

## Findings and disposition

**A — stale expected failure after another consumer recomputes a query (fixed).**
`specialize_alias` and `specialize` used the prerequisite query's `NotStarted`
state as an invalidation marker. A class could complete, another template could
warm that query, and the first consumer would keep its old failed answer.
The independent `alias_other_consumer` and `result_other_consumer` reducers in
[audit66_controls.py](../student.tests/pa18/audit66_controls.py) both fail at entry
and pass at the reviewed tip. They test immediate substitution after completion,
with no instantiated definition changing meaning: N3485 14.8.2 [temp.deduct]/7–8
and 5.3.3 [expr.sizeof]/1 (`doc/n3485.txt`). Compiler agreement is supplementary,
not the language proof.

The query dependency owner now advances a **per-query revision** only for
invalidated facts. Each failed alias/signature has one typed prerequisite slot
containing QueryId and observed revision. Reuse compares these identities even
if the query has been recomputed; retry updates the same slot. The existing
reverse-edge worklist and on-demand retry remain. There is no global generation,
registry scan, cache flush or all-specializations retry. At 32/128/512 pending
classes, completing and warming one class invalidates exactly one query in all
three cases; edges are exactly 32/128/512. Unrelated failures remain cached.

**B — semantic evaluation in an ordering inspection walk (fixed).**
The used-parameter completeness walk called `value_argument_id`, whose contract
includes query evaluation and constant canonicalization. That is a broader owner
than structural partial-ordering inspection requires. It now walks tagged,
retained QueryIds directly with a visited set, including decltype/array-bound
children. No query result, class completion or constant evaluation is demanded
by this walk. All 64 ordering/pack controls and the accumulated fixtures pass;
this closes the spec §4/§9 architecture finding even where output was unchanged.

**C — reference correction (accepted with proof).**
[reference-correction65.md](reference-correction65.md) identifies pinned bundle
`c2f713cd70d06170632bfde3e75dd6fe1aa44d98` and the exact reducer. N3485
3.6.2 [basic.start.init]/2 (`doc/n3485.txt:3841`) requires the reference's constant
initialization before any dynamic initializer. 5.19 and 7.1.5 establish that
its constexpr conversion/selected conditional arm meet that obligation. The
pinned reference instead installs a zero reference dynamically; the earlier
initializer's read fails. Re-execution confirms reference failure and student
exit zero. The corrected address initializer and removed obsolete dynamic body
preserve the backing object's identity, required behavior, all 420 inputs,
status sidecars and canonical comparison rules. No additional reference changes.

## Architecture and optimization trace

The recorded trace combines an ordinary `main`, a conversion-function template
`Source::operator T() const`, and `pick(T)`/`pick(T*)`. Source buffers live in
the TU preprocessor; `PostTokenCursor`→syntax cursor feeds the parser and semantic
callbacks, without owning whole intermediate token streams. Parser changes in
this range continue the existing specifier region and restore mode around
special-member bodies; instantiation never replays grammar.

The retained source graph supplies TypeId/EntityId/QueryId identities.
`template_conversion_deduction` deduces `int`, establishes immediate signature
facts through immutable frames, and records object/result conversions.
`template_ordering` compares the nominated pointer parameter shapes and records
the selected `pick<int>(int*)`. The cache key includes both original TypeIds,
argument-count/context, operator object owners and conversion context. No names,
rendered types, manglings or serialized expressions are semantic equality keys.

`template_instantiation` observes monotonic specialization body state and
projects compact source/context occurrences; `syntax/occurrence.cpp` shares
source nodes and retains only required occurrence/topology metadata. Nondependent
facts reuse their source owners; class completion, defaults, bodies and storage
have distinct demand owners. Member-head substitution builds immutable prefix
frames from retained parameter types instead of reparsing or copying environments.
The reviewed query dependencies observe incomplete class edges and now preserve
validity independently of subsequent scheduling.

`lowering/driver.cpp` directly constructs a typed `lowir_model::Program` from
these facts. The trace has exactly one selected conversion body and one selected
pointer overload; the unused generic overload is absent. Typed ABI nodes encode
`_ZNK6SourcecvT_IiEEv` and `_Z4pickIiEiPT_`. The ordinary emission enumeration is a
single pass, not repeated semantic recovery. Text is written only at PA18's
required LowIR boundary; full validation runs only under the explicit audit flag.
The supplied native backend executes this LowIR to ELF with checked exit zero.
**Native selection/allocation/ELF ownership is PA24+, not claimed here.**

A useful fact in this range is the constexpr reference's selected conversion and
object address. The semantic constant-address owner consumes those recorded facts;
static lowering emits the address relocation and does not schedule the obsolete
dynamic action. The proof is the C++11 static-initialization obligation, not an
optional optimization's profitability decision. Runtime conversions and calls in
the ordinary trace retain their O0 calls, effects, ordering and ABI. The benchmark
loops use volatile bounds and checked results, covering calls, memory and floating
point. No optional transformation, fixed-point pass, inlining or speculative code
growth is introduced. Missing proofs retain the existing conservative O0 path.
Own-native spill/loop optimization and self-host workloads remain PA24–PA34 work.

Storage is explicit: canonical types, entities, queries, dependency edges and
frames use TU-owned vectors/flat indexes; scratch candidate bindings and worklists
release on return. No per-node owning shared pointers or process-global mutable
cache is introduced. Frontend/semantic/ABI scratch releases at the TU loop boundary;
the output program/linkage facts survive only until LowIR output. Completed
lookups are O(1) average; work follows required candidates, type/query occurrences,
pack lanes and dependency edges. Graph growth is bounded by demanded facts,
observed edges, and output. See [performance66.md](performance66.md) for measured
latency/RSS, executable runtime/size, scaling, A/A calibration and ABBA observations.

## Validation and remaining work

| Check | Reviewed-tip result |
|---|---|
| `make test-pa18` | **327/420**, exit 2; exactly the entry's 93 failures, including the same 69 status and 24 LowIR failures. |
| Required prior-through command (`n=18; … make test-report-through-pa$((n - 1))`) | **2609/2609**, exit 0. |
| `perl scripts/cppgm_file_audit.pl --stage pa18 --paths dev/src` | Pass, exit 0; same three inherited header-division advisories. |
| Coverage/comparison audit | All 420 fixture hashes retained, no new fixture/harness/status changes since entry; only the proved reference change since stage base. |
| Explicit personal controls | 64 ordering + 33 substitution + 51 conversion + 5 new cache controls; all pass. New controls all fail on frozen entry. |
| Direct ABI | 10 sizeof-pack tool/API cases, 8 increment/decrement tool/API cases, 4 source conversion encodings; all pass. |
| Accumulated course controls | 63 pass: 58 native executions and 5 validated-LowIR cases whose source supplies no complete executable. Original nonzero expected results are preserved. |
| Architecture/control trace | Validated LowIR, exact selected ABI identities and checked ELF execution; reference reducer independently reproduced. |

Root report invocations share `.test_counts`. The first concurrent validation
attempt produced mixed aggregates; its logs are retained as superseded evidence.
Both required report commands were rerun sequentially, producing the totals above.

The checkpoint gate is preservation, not full-stage success. Remaining required
work is grouped in [plan.md](plan.md): retained lexical/member/pack contexts and
expression validity; constructor/explicit deduction and NTTP value identities;
LowIR initialization/result facts. No required failure or comparison is waived.
Passing runtime for a LowIR mismatch does not replace the course oracle. Earlier
handoffs deferred review of one interconnected default/query/member path and
repeated evidence packaging; those avoidable boundaries should be consolidated.
Full through-PA18 must pass before advancement to PA19.

| Checkpoint ledger | Range / fixes | Evidence / disposition |
|---|---|---|
| 66, first accumulated audit | `94dcb8ad` → `3a883d10`; all three handoffs plus local query-revision and observational ordering fixes | PA18 327/420, same 93 failures; prior 2609/2609; file audit pass; coverage preserved; performance accepted within PA18/O0 scope. Reviewed code tip recorded above; remaining implementation broadly grouped, stage advancement pending. |
