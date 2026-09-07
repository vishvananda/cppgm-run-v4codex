# PA4 implementation plan

Stage base commit: `a682ffe75533c8aed941f46f6131c9e8af22f93d`
Last reviewed commit: `a682ffe75533c8aed941f46f6131c9e8af22f93d`

Target: PA4 full-stage. Entry: 0/105; implemented: 105/105, earlier stages 100/100. Previous goal
turn: progress (PA3 committed and verified); PA4 starts from the scaffold.

## Design and remaining groups

| Owner | Data flow / work / validation |
| --- | --- |
| Macro cursor | Immutable sources → borrowed compact PP tokens → deferred definitions/arguments and iterative rescan → shared PA2 cursor. Dense identifier-indexed definitions; token-local ancestry and permanent unavailable paint. Validate all macro fixtures, prescan sharing, rescan boundaries and rejection. |
| Directive cursor | Streaming file frames and one directive at a time; per-file conditional stacks feed PA3 directly. Includes share TU definitions; presumed locations and device/inode once state reset per primary. Validate directive fixtures and multi-source reset. |
| Observation adapter | `preproc -o` writes PA2 records directly from the structured cursor; invalid posttokens reject. No text transport between phases. Prior PA1–PA3 and full through-PA4 validate integration. |
| Evidence | Optional existing-work telemetry, fixed frontend scaling workloads and latency/RSS measurements. No executable generation at PA4; executable runtime/text N/A. No optimization benefit claimed against an unimplemented baseline. |

Source buffers and names live through the TU; expansion scratch lives only for
deferred work. Retokenization is restricted to language-required token pasting.
Work should track source bytes and expansion output, with argument prescan once
per used argument, not once per occurrence. Later semantic/IR phases remain
structured consumers, with no premature syntax or backend representation.

## Handoff ledger

- Entry: read instructions/spec/handouts and unchanged fixtures; recorded review
  markers before stage edits. Implement all related groups, then run personal
  checks, performance evidence, `make test-pa4`, through-PA4 and file audit.
- Handoff reason: implementation in progress; no stopping boundary reached.
- First implementation: all 105 unchanged PA4 fixtures pass; prior report
  100/100 and file audit 43 files pass. Shared pull-token interface, indexed
  definitions/parameters, fixed-depth persistent ancestry, cached argument
  prescan, directives and file identity are implemented. The run target also
  confirms multiple-primary output. Remaining: personal depth/lifetime checks,
  allocation/scaling evidence, and final cumulative validation.
- Allocation review found repeated copying in nested argument prescans (1000
  levels used 314 MiB). Replaced it with indexed borrowed slices and an explicit
  task stack; ordinary substitution copies only produced tokens. Generated
  spelling storage now rewinds when the expansion drains. Course 105/105 and
  163 personal cases pass; sanitizer/API checks are running.
- Performance campaign budgets, before A/B measurement: unchanged-workload
  paired latency regression <=10% plus A/A noise, peak RSS <=15% plus 1 MiB;
  host-tool text growth <=15%. Fourfold frontend input growth must take <6x
  latency and <5x RSS. Indexed nesting captures <=3n tokens, uses O(n) scratch
  with bounded host call depth; generated counter spelling stays <=128 KiB.
  Frozen A is `28279a9d0`'s ordinary binary. No generated-executable surface
  exists yet; runtime and generated text size remain N/A.
- Depth/lifetime increment: 163 personal semantic cases pass normally and with
  ASan/UBSan; direct API checks validate presumed/physical locations, stable
  identifier identity, 20,000 nested prescans and 100,000 counters with <=128 KiB
  spelling storage. All 105 course cases also pass under ASan/UBSan. The final
  frozen campaign and completion audit remain; no related work is handed off.
