# PA29 implementation / handoff155

Target: **PA29 full-stage**. Phase: implementation.
Stage base commit: `2734e5c67eaa7c0cf4bbbd510dba8d60f36d6543`.
Last reviewed commit: `2734e5c67eaa7c0cf4bbbd510dba8d60f36d6543`.
Entry: clean; 182/403 passing, 221 failing. Previous PA28 audit completed;
this entry is progress through inspection of authoritative code and failures.

## Design and remaining groups

| Owner/group | Data flow and complexity | Validation |
|---|---|---|
| Driver + streaming preprocessor (active) | Built-time hosted config and ordered flags → shared token cursor → explicit PA4 posttoken view; source-linear work plus expansion/include edges. Probes must use actual builtin ownership. | PA29 preprocessing, explicit negative/ordering controls, PA1–28 |
| Hosted scalar forms/parser concessions | Source tokens → retained typed syntax/semantic facts; no hosted-only lowering. | Compile fixtures, literal/layout controls |
| Builtin traits/intrinsics | Canonical type/call facts → typed LowIR → existing native backend; registry-backed feature claims. | Compile/run fixtures and LowIR checks |
| Templates, layout, ABI and hosted wrappers | Existing shared semantic/demand owners; extend retained facts instead of replay/name recovery. | Remaining compile/run clusters |

## Performance and acceptance

Apply spec §9 stage-scoped acceptance: measure compiler latency/peak RSS and
runtime/text size for executable controls. Freeze binaries/flags/inputs; retain
A/A and ABBA observations for equivalent correct paths. New semantic capability
costs are reported separately. No optimization speedup or unsupported blanket
percentage gate is assumed. Keep inherited measurements and mandated limits.

## Handoff ledger

- Implementation unfinished: all groups above; first boundary is the complete
  hosted preprocessor group, extended into directly related defects as evidence
  supports. A minimum count improvement is not a stopping criterion.
- Independent review pending: whole-stage source-to-ELF/spec audit; no reviewed
  marker advances during implementation. This is distinct from known missing
  implementation and does not waive it.
- Required final evidence pending: stage test, prior through report, file audit,
  explicit controls, performance, coherent commits and clean status.
