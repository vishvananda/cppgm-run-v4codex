# PA22 implementation plan

Target: **PA22 full-stage**. Phase: **implementation**.
Stage base commit: `a8482d768bd2dcede42ea63ef39e39cf3245c380`.
Last reviewed commit: `a8482d768bd2dcede42ea63ef39e39cf3245c380`.
Entry: **22/99 passing, 77 failures**; PA1–PA21 reported passing. No prior
implementation process survives the interrupted entry turn.

## Design and remaining groups

- Member-pointer representation/application/conversion: semantic canonical
  types, selected declarations and conversion/object facts feed typed LowIR;
  static/constexpr values preserve member identity. Lowering owns the course
  representation, null handling and receiver adjustment. Work is constant per
  operation plus required base paths; no repeated lookup or text transport.
- Dependent member types, addresses, NTTPs and packs: type formation,
  substitution/deduction and constant owners must share canonical facts and
  retained parsed regions. Extend this group after the common runtime path.
- Multi-base lookup/access and pointer projection: indexed base edges and
  recorded layout/conversion facts own ambiguity and adjustment. Generated
  lifecycle actions preserve base order.
- Single-vptr `dynamic_cast<void*>`: recorded RTTI demand and typed lowering;
  broader virtual layouts remain PA23 work.

Validate each group with existing contract cases and explicit personal tests
in `student.tests/pa22/`; finish with root stage/prior reports and file audit.
Preserve every contract fixture and comparison rule.

## Performance evidence

Freeze entry/final binaries and workloads. Record compiler latency/peak RSS,
and supplied-backend runtime/text size for executable cases. Use A/A and ABBA
observations for claims, preserve samples and check equivalent outputs.
PA22 is O0 correctness work: no optional optimizer is planned. Historical
self-selected targets remain diagnostics under the spec's stage-scoped policy;
all mandated limits and architecture requirements remain applicable.

## Handoff ledger

114 entry: failure ownership and authoritative 22/99 baseline established;
implementation unfinished. Independent review has not examined PA22 changes.
The two review markers above remain fixed throughout implementation. A later
handoff must record implemented groups, remaining defects, validation and the
concrete boundary; an implementation handoff does not certify full-stage audit.
