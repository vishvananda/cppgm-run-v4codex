# PA5 independent validation

Run personal tests explicitly; the course harness does not discover this folder.

```sh
python3 student.tests/pa5/check_core.py
python3 student.tests/pa5/check_extended.py
python3 student.tests/pa5/check_audit.py
python3 student.tests/pa5/build_checks.py /tmp/pa5-sanitize --sanitize
ASAN_OPTIONS=detect_leaks=1 python3 student.tests/pa5/check_graphs.py /tmp/pa5-sanitize/api
ASAN_OPTIONS=detect_leaks=1 python3 student.tests/pa5/check_audit.py /tmp/pa5-sanitize/compiler
python3 student.tests/pa5/check_course_binary.py /tmp/pa5-sanitize/compiler
perl scripts/cppgm_file_audit.pl --stage pa5 --paths dev/src
make test-report-through-pa5
```

The audit adds 19 syntax-choice regressions. The direct graph runner validates
181 successful course translation units (including companions), those 19 audit
units, literal ownership after cursor destruction, physical/presumed ranges,
cross-file concatenation, import cycles, category updates, anonymous namespace
identity, and bounded nested-angle/identifier-hint work. Sanitizer errors fail
the runners even for fixtures whose contract requires rejection.

The fixed benchmark generator predates the audit. `audit_performance.py` runs
ordinary compilation with four identical primary operands per process (eight
for nested inputs), two A/A pairs, B/B, two ABBA blocks and eight startup probes.
It pins the process and children to one permitted CPU. Separate runs record
phase/work counters; ordinary/stats ABBA comparisons quantify instrumentation
overhead. No build or test should overlap a campaign. Raw JSON contains every
observation and exact input/output/binary hashes, including noisy observations
and regressions. Verification checks identities, output equality, the protocol,
startup separation, predeclared budgets, scaling and fresh TU state.

```sh
python3 student.tests/pa5/audit_performance.py measure <frozen-A> <frozen-B> /tmp/pa5-final.json
python3 student.tests/pa5/audit_performance.py verify /tmp/pa5-final.json <frozen-A> <frozen-B>
python3 student.tests/pa5/audit_performance.py report /tmp/pa5-final.json
```

Final A is the ordinary build at `7e8d10bf2`; final B is `f0a0f614a`. Rebuild in
isolated checkouts with the recorded host toolchain and ordinary dev flags,
then copy/freeze the executables before measuring. Final committed observations
are `final-audit-performance.json`. The `9cfce7949` candidate used reusable
indentation; `reuse-candidate-performance.json` preserves its full campaign.
`indentation-performance.json` compares that candidate with a control differing
only in indentation storage, on fixed classes/expressions inputs. The control
can be rebuilt after an ordinary dev build:

```sh
python3 student.tests/pa5/build_indentation_baseline.py /tmp/pa5-indentation-control
python3 student.tests/pa5/audit_performance.py measure-indent /tmp/pa5-indentation-control/no-reuse <frozen-9cfce7949> /tmp/pa5-indentation.json
```

This experiment did not establish repeatable profitability and the optional
change was removed. Historical `performance.json` and `indexed-performance.json`
remain unchanged; use their matching binaries with `verify_performance.py`.
[The stage performance record](../../pa5/performance.md) explains all four
performance dimensions, applicability, spreads and decisions.
