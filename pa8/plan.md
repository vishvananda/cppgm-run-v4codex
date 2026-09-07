# PA8 final plan and ledger

Target: **PA8 full-stage**. Phase: **complete; independently audited**.
Stage base: `7e4297484f117980a3f1d55f43932d6bda3a8cc1`.
Last reviewed implementation: `bc2cd043d`; benchmark/protocol: `2f21a26ac`.
Incoming checkpoint: `7313af05a`; all intervening handoffs have been reviewed.
Entry: **0/109**. Final: **109/109**, cumulative **793/793**, **8/8 stages**.
The incoming 811/811 summary was stale: its own primary log and the fresh root
report both say 793/793 (684 earlier +109 PA8); no coverage was changed.

## Final Spec Alignment

| Owner | Final architecture and outcome |
| --- | --- |
| Model / builders | Inline canonical types, interned names, typed IDs and 14 flat geometric pools. Direct construction shares the writer. Constant-time slice checks now reject interleaved function/block/slot ownership. |
| Reader / writer | One borrowed token view per immutable input -> typed records -> deterministic output. Cross-file symbols resolve once; declarations, metadata, instruction/CFG order and literal values survive. Text is an explicit interface, never production phase transport. |
| External validator | One handler discovery scan, one full instruction traversal, one ordinary-edge sort/deduplication and stamped predecessor checks: O(IR + E log E), O(IR + E) scratch. Phi rejects f80 and all handler blocks, including later cleanup/try registrations; facts are recomputed after model edits. |
| Exercises / executable boundary | Sum domain proves i64 arithmetic; swap preserves aliases; both indirect calls retain order/effects. The external harness alone uses the supplied native backend. No optimizer, owned encoder or unsupported source-to-ELF path is introduced. |
| Inherited frontend | Independently traced parser/semantic construction and canonical declaration-only template demand. Source and entry point are unchanged from PA7. Current template benchmark, work facts and prior-stage/API checks pass. |

[Final audit](audit.md) records the full source review, representative traces,
findings, fixes, validity, lifetimes and pipeline budgets. C++ lowering,
template-body substitution, MIR/native optimization/encoding and self-hosting
remain the explicit later-stage boundaries, not unfinished PA8 groups.

## Performance and validation

[Final performance report](performance.md) contains frozen incoming A versus
final B, AAAA noise calibration and two ABBA blocks on a pinned CPU, equivalent
outputs, compiler latency/RSS and executable runtime/text together. All 200
primary and 14 construction/adapter observations are retained. The unchanged
frontend has a separate 70-observation template-demand run. All budgets pass.

Mean paired compiler wall changes are −0.76% to +2.87%; largest median RSS growth
is 18 KiB; compiler text +1.43%. Fourfold inputs scale 3.87–3.98x wall and
3.69–3.87x RSS. The handler checking cost is mandatory and bounded; no runtime
improvement is claimed. All four executable pairs are byte-identical at
234/250/270/417 text bytes. Sum timing differences and all noise are disclosed.
Template wall/RSS scale 4.00x/3.23x with unchanged specialization work.

- `make test-pa8`: **109/109 pass**.
- `make test-report-through-pa8`: **793/793 pass; all eight stages pass**.
- `perl scripts/cppgm_file_audit.pl --stage pa8 --paths dev/src`:
  **pass; 97 files, no warnings**.
- Personal checks: **24 valid +56 invalid**, writer fixed points and CLI;
  direct API, complete sum domain, alias/callback/floating native checks pass.
- Final standalone **ASan/UBSan + leak checks pass**; inherited PA7's 63 audit
  cases and source-graph/selected-fact/demand API pass.
- Both fresh benchmark verifiers check current artifacts/protocol/work/budgets.
  Historical JSON remains intact; absent old `/tmp` artifacts were not reused.

## Consolidated ledger

- `da047b4a9`: initial plan; `66167cf72`: complete LowIR implementation;
  `01d39a2f6`: literal/shape hardening and telemetry; `7313af05a`: checkpoint
  ownership/performance record. This audit closes all four handoffs.
- `bc2cd043d`: fix phi handler/type legality and builder slice ownership;
  add semantic and API/edit probes. All implementation changes stay in `dev/`.
- `2f21a26ac`: prospectively freeze final corpus/budgets and verify provenance;
  add the unchanged frontend's template benchmark wrapper.
- Final consolidation commit: this plan, independent audit, performance report,
  reproduction instructions and all raw final observations; required gates
  rerun against final state and clean committed status checked afterward.
- Remaining PA8 groups / unaudited PA8 handoffs: **none**. No PA9 advance;
  no course fixtures, references, harnesses or prior implementation changed.
