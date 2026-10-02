# PA33 implementation plan

Stage base commit: 676b6e328c7a05334b15f1c9ee1ec30c783e9aff
Last reviewed commit: 676b6e328c7a05334b15f1c9ee1ec30c783e9aff

Target: **PA33 full-stage**. Phase: **implement**, handoff 219 in progress.
Entry report: 47/73 passing, 26 failures including controls not completed after
the first failure; ten reported MIR envelope failures. Prior stages pass.

## Design and remaining groups

Keep the PA32 typed LowIR pipeline and function-local MIR/direct ELF backend.
O0 preserves its baseline. O1–O3 improvements must keep ABI/clobbers, volatile
effects, unwind edges and debug provenance accurate in the encoded MIR.

| Group / owner | Data flow and proof | Work / validation |
| --- | --- | --- |
| Register placement (`native/placement`, `selection`, `carry`) | Typed value uses, edges and fixed clobbers → live locations → actual instructions | Bounded function-local census/placement; bulk setup, cross-block copies, loop/EH loads, branch/cycle pressure |
| Frame layout (`native/placement`, `layout`) | Nonoverlapping private value lifetimes → reused homes; escaped/volatile storage retained | Linear or near-linear with conservative fallback; volatile frame and reused-home fixtures |
| Builtin call facts (`native/calls`, encoding, dump) | Explicit runtime identity + compatible signature + level → bounded strlen prefix selection | Constant work/growth per admitted call; native controls, page boundary reducers and runtime measurements |

Inspect related cases together and extend each group while its ownership proof
supports further work. No fixture/reference/comparison changes planned.

## Performance and acceptance

Freeze entry/final binaries and inputs. Record compiler wall time/peak RSS and
checked executable runtime/text size, A/A calibration and repeated ABBA pairs.
Mandated fixture bounds and implemented work/growth limits are gates; inherited
ad hoc timing ratios remain diagnostics under spec.md stage-scoped acceptance.
No benefit is claimed from static MIR counts alone. Record pass budgets and
measured costs when implementation is concrete.

## Handoff ledger

- Entry 219: clean HEAD recorded above; no live compiler/test process remained.
  Initial inspection is evidence-changing progress; no prior implementation
  turn in this stage is available to classify beyond that observation.
- Unfinished implementation: all three groups above and full required checks.
- Independent review: whole-stage architecture/performance/ABI/debug review
  remains required after implementation; markers above are not advanced here.
- Handoff boundary: not yet established. Required final evidence: PA33 course
  and debug, root through report, file audit, explicit personal checks, committed
  intended changes and clean status.
