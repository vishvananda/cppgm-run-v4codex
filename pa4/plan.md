# PA4 implementation plan

Stage base commit: `a682ffe75533c8aed941f46f6131c9e8af22f93d`
Last reviewed commit: `a682ffe75533c8aed941f46f6131c9e8af22f93d`

Target: PA4 full-stage. Entry: 0/105, earlier stages 100/100. Previous goal
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
