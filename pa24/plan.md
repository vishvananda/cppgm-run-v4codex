# PA24 implementation ledger

Stage base commit: bde9eb3e128e24923a1de40bb63b8e348a13553b
Last reviewed commit: bde9eb3e128e24923a1de40bb63b8e348a13553b

## Design / completed implementation group

Scalar native execution foundation: PA8 typed Unit -> function-owned placement
facts and flat MIR -> shared MIR view/x86 encoder -> typed fixups and direct ELF.
No host/reference compiler, assembler, text transport or fixture recognition.
Unit tables retain compact IDs; placement/MIR die after each function. Reserved
GPR/XMM scratch effects, actual callee saves and frame policy drive encoding.

This group covers integer/pointer widths through 64 bits, scalar loads/stores and
conversions, direct compare/branch/switch, parallel phi edges, scalar direct and
indirect calls (including stack/by-address arguments), hooks, globals/relocations,
fixed bulk copies/zeros, scalar atomics and canonical strlen runtime support.
Related extensions include parameter-slot promotion, alias lifetime sharing,
safe indexed operands, bounded forward-edge retention and immediate store-reload
carrying. Floating global data encoding is tested; floating execution is unfinished.
The LowIR validator now accepts the course's integer consumption, by-address
actuals and bounded slot reads, retaining strict phi and pointer-parameter checks.

Work bounds: six linear body walks, three value walks, one CFG walk; nine register
probes per placement; O(E log E) edge ordering; at most two phi transfers per input.
Repeated switch cases share one phi transfer block. Zero-form costing inspects
at most 32 bytes. No fixed-point scans, inlining or unrolling. O1/O2/O3 currently
use the conservative O0 policy; later optimizer work remains stage scoped.

## Unfinished implementation (all requirements retained)

- ABI/value classes: XMM f32/f64, x87 f80, i128, multi-eightbyte objects/results,
  variadic save areas and wider atomic operations. Owner: native selection/ABI.
- Runtime/storage: TLS, dynamic stack and EH/runtime instructions required by the
  remaining course inputs. Owner: target runtime/layout, consuming typed IR facts.
- Canonical MIR policies: `strict/100-object-abi-lowered`,
  `structural/200-stack-arguments-beyond-six`,
  `structural/800-single-edge-callee-saved-retention`; these execute correctly but
  still require their mandated dump shapes. Owner: call/result/frame placement.
- Extra behavior controls: scratch-carried-frame-reloads is blocked by floating
  execution; deferred-address-parameter-carrier-reuse still needs its required
  parameter carrier/home relationship. These are implementation work, not waivers.

Handoff boundary: executable scalar foundation plus bounded selection. Further
expansion requires split/XMM value locations and coordinated ABI call/frame
classification. The remaining canonical frame/result policies share that owner;
printing unused fictitious frame facts would violate the MIR/encoder invariant.
Resume with that coherent ABI group, not per-fixture rendering patches.

## Performance evidence

See `student.tests/pa24/README.md` and `performance.md` for frozen A/B inputs,
raw observations, paired ABBA results and A/A calibration. The initial scaffold
cannot be a runtime baseline. All historical measurements remain available.
Diagnostic budgets are <=15% compiler latency/RSS increase, no text growth for
forward-edge retention, and a repeatable affected-runtime benefit. These are not
extra course exit gates. A measured global-address spill regression was removed;
rematerialization restores the memory benchmark's original executable bytes.

## Handoff ledger / independent review

- Previous interrupted turn: no verifiable PA24 implementation; clean scaffold at
  the stage base. This turn made authoritative implementation/test progress.
- `f9ff5dd4`: typed scalar backend and shared contract validation, 199/296.
- `7d1be282`: bounded placement/phi/bulk selection, 221/296 plus 14/14 controls.
- Final handoff: current validation and evidence recorded below; no test/reference
  changes, coverage cuts or comparison-rule changes. Personal tests do not count
  toward the progress gate. The 435-item initial inventory includes 125 excluded
  design regressions; the unchanged course oracle denominator is 296 plus controls.

Independent audit remains due: trace scratch effects and alias carrier lifetimes,
phi-edge parallelism, shared validation compatibility, typed fixups/MIR fidelity,
and function-state release/work bounds. These are review questions; the explicit
unfinished implementation above is separate and cannot be cleared by this handoff.
Review markers remain unchanged. This is not whole-stage completion or advancement.

Final evidence:
- `make test-pa24`: 221/296 oracle cases, versus 0/296 at entry; 14/14 focused
  controls. The stage still fails: 72 missing-feature cases and three MIR policies.
  All 223 successfully compiled positive fixtures match runtime exit/stdout even
  when their MIR comparison fails. Extra fixed-zero control passes; two extra
  controls remain unfinished as listed above.
- `make test-report-through-pa23`: 3856/3856; file audit: pass, three inherited
  warnings. Personal arithmetic (1820), integration and runtime-outcome audit pass.
- Frozen current B: compiler ratio 0.959 (no speedup claim), RSS essentially equal;
  forward-edge runtime ratio 0.592 with 176 -> 161 text bytes. The corrected
  memory workload is byte-identical to A. Raw spread and historical regressions
  are retained in the performance report. No unsupported extra gate is imposed.
