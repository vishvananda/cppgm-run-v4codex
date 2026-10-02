# Final audit 218 evidence

Reviewed implementation: `dcd298afff36573014188a2c3cee826f2500cbf8`.
Entry: `5a767f2053b83c830c287263a58c732a05965b5c`.
Previous checkpoint: `401519044a2c28130d4085b3bc7c3d0411b5dc8f`.
The [audit](../../../pa32/audit.md) and [plan](../../../pa32/plan.md) state the
complete architecture, source-debug corrections, proofs, limits and acceptance.
No implementation or unaudited handoff remains for PA32.

`binding.json` binds all tracked implementation files, audit scripts and owning
documents; all retained artifact files, binaries, input/output/check logs; course
fixture trees; and these evidence records. Scratch and frozen binaries remain
under `$RALPH_ARTIFACT_DIR/pa32-218` on the shared volume. Generated objects,
executables, logs and `.my*` files are not committed.

`checks.json` preserves exact commands, statuses and log digests for every
attempt. Last results: 32 required checks pass, including course 219/219, through
5397/5397 across 32 stages (also the count in the supplied primary log, copied
as `entry-primary.log`), file audit, required PA32 direct/source debug and
25/25 debug replay. The four file-audit header warnings are recorded. A historical
personal loop-trace assertion failed because it demanded a particular unroll in
debug mode; its second attempt checks g0 profitability and both modes' runtime,
metadata, MIR and object identity. Range traces similarly cover both modes.
Neither required fixture outcomes nor direct-IR debug coverage were relaxed.

The exploratory `prior-debug` row exits 2 on five PA8 exact shape expectations;
`prior-debug-entry.log` reproduces the same failures on entry cppgm/lowiropt.
The native tool used in both is unchanged by this audit. PA8's README excludes
these later source/optimizer/native debug surfaces. The owning PA32 debug check
passes. This diagnostic and the runner's initial assertion logs are preserved;
they are not silently counted as successful checks.

[performance.md](performance.md) reports all four dimensions, paired ratios and
spread. Twelve final lanes contain **2156 observations** with frozen compilers,
flags and inputs, CPU 2, A/A and six ABBA blocks. Each executable result is
checked; host g++ only links compiler-generated code. Telemetry on affected
inputs is collected outside timings; common variants use the same stats flag.
`debug.json` is a final g0/line-table cost comparison, not a speed claim against
the incorrect entry debug implementation. `selfhost.json` compiles a fixed
compiler component and has no runnable entry; full self-hosting belongs to PA34.
`owner-objects.json` verifies that all 24 affected final B objects are identical
to the previously measured accepted owner outputs. Native disassemblies, actual
MIR traces and all output images are in the artifact binding.

`historical-common-o1.json` and `historical-selfhost.json` preserve **252** early
audit measurements of the intermediate implementation, before the final line
policy. Their separate source/binary hashes and failed debug reports remain.
`history.json` verifies **4564** observations from 215–217, **642** source
bindings and **2047** artifact bindings against their own source commits. The
215 mutable dev binary paths are explicitly rebound to identical-hash frozen
artifacts; this does not substitute current binaries for historical ones.
Earlier reviewed evidence 210–214 and every rejected policy remain in the repo.
No outlier, negative experiment, mandated limit, fixture or comparison rule was
removed. The audit adds no reference correction; the inherited even-stride
correction retains its [LowIR proof](../../../pa32/reference-corrections.md).

Reproduce required and personal checks:

```
python3 student.tests/pa32/audit218_checks.py NEW_CHECK_DIRECTORY
python3 student.tests/pa32/audit218_performance.py NEW_PERFORMANCE_DIRECTORY
```

The performance runner expects sibling `baseline/` and `final/` frozen binaries
and the bound stage-owner A binaries. It supports `--resume`; a successful
record is reused only for that experiment directory. Use a new directory when
binaries, flags or inputs change. Re-run the historical verifier independently:

```
python3 student.tests/pa32/audit218_history.py OUTPUT_JSON
python3 student.tests/pa32/audit218_records.py verify --clean
```

The final verifier checks required latest results, every historical attempt's
log hash, frozen sources/binaries/artifacts, all A/A/ABBA arithmetic, owner-object
identity, inherited 215–217 source/artifact bindings, unchanged course trees and
the final reviewed boundary. It does not treat timings or personal pass-choice
counters as extra exit gates.
