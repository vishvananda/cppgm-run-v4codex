# PA6 final plan and ledger

Target: **PA6 full-stage**. Phase: **complete**.
Stage base: `9249196518f45492822fb2e3da4eb5d82af0ed13`.
Last reviewed implementation: `85011bf5a`.
Entry: 0/105. Final code: PA6 **105/105**, PA1–6 **498/498**.

## Final Spec Alignment

The [independent audit](audit.md) reconstructs the streaming source/token path,
parser/semantic callback, one source-faithful graph, canonical entities/types,
indexed lexical/qualified lookup, complete-class body demand, constant/layout
facts and TU release boundaries. It traces a compound declaration, a retained
template parameter environment/body and a demanded sizeof fact. There is no
text phase transport, semantic tree copy, global retry or per-node owning
allocation. Source views and canonical semantic identities remain distinct.

PA6 has no instantiation, LowIR, executable optimizer, native encoder or ELF
surface. Runtime/text and self-hosting are N/A here; later stages must consume
these typed facts and establish their own legality, work and code-growth bounds.

## Findings and changes

`85011bf5a` closes the independent findings across their owners:

- One alias declaration path and canonical nested function types, retaining
  declaration source forms and object array-completion views.
- Correct parser/semantic owners for qualified class/enum definitions and enum
  base types; required rejection of definitions in sibling scopes.
- Constructor function-try body preservation and control/branch block lifetime.
- Whole-inline-set qualified lookup, indexed/deduplicated using edges, and
  inline adjacency that avoids scanning irrelevant ordinary directives.
- Checked class-layout padding/addition and class-only constructor demand.

The independent audit changed no handout, course fixture, reference, comparator,
discovery, timeout or source registration. All seven stage semantic sources
are registered in the shared tool list.

## Performance and validation

The [performance record](performance.md) identifies the exact final frozen
binary and all 412 observations across the two campaigns; both complete
verifiers pass. Signature workloads improve 19–23% versus first PA6. The edge
corpus improves 75% / 92.4% versus the checkpoint, with 4.034x wall scaling for
4x input. Other semantic pairs range from 0.7% faster to 3.9% slower than first
PA6. Largest template RSS is 56,078 KiB (+12.4%); host compiler text is 277,702
bytes (+33.18% over PA5 / +3.04% over first PA6). Executable runtime/text are
N/A. Historical checkpoint data and failed/interrupted campaigns remain retained.
Budgets are unchanged: wall <=10% + A/A noise; RSS <=15% +1 MiB; host text
<=35% over PA5 / <=5% over first PA6; 4x input <6x wall / <5x RSS +1 MiB.
The original 25% feature-text forecast had been revised before the checkpoint
campaign, not in response to this final audit. No failed memory gate was relaxed.

Final code passes `make test-report-through-pa6` (498/498, six stages) and
`perl scripts/cppgm_file_audit.pl --stage pa6 --paths dev/src` (71 files).
ASan/UBSan/leak builds pass both graph APIs, all 57 PA6 personal cases and all
105 unchanged PA6 fixtures, including multiple primary translation units.
Inherited PA5 personal checks pass: 10 core, 15 extended, 19 audit cases.

## Handoff ledger

| Commit / handoff | Independent review outcome |
| --- | --- |
| `f6c4ddf33` | Baseline marker and initial dispatcher-stub failures reviewed. |
| `5749f43b4` | All seven semantic source owners, graph extension, type construction and driver reviewed. |
| `78df00b67` | Anchored lookup, enum transitions, signature/constant caches and class layout traced. |
| `3f6de92f5` | Body queue intervals, declaration points and definition identities rechecked; missing function-try and statement scope paths corrected. |
| `1edcbe5db` | Class-only demand arena and packed records reviewed; no language fact removed. |
| `15c7f1fcd` | Historical performance verifier rerun using rebuilt, hash-identical baseline binaries and the retained checkpoint binary. |
| `85011bf5a` | Independent fixes and 24 added behavior cases, expanded identity API, edge benchmark and stronger performance verifier committed. |
| Final evidence | Original 20-workload and added 2-workload frozen campaigns pass every verifier; both required exit gates pass; final audit/evidence committed and worktree checked clean. |

No earlier handoff remains unaudited. Remaining PA6 work: **none**. Final
completion is recorded only after committing this evidence and confirming an
empty `git status --short`. No generated objects, execution logs or `.my*`
outputs are committed.
