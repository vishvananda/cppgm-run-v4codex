# PA7 personal validation

Run explicitly from the root:

```sh
python3 student.tests/pa7/check.py
python3 student.tests/pa7/check_audit.py
python3 student.tests/pa7/check_course.py /path/to/alternate/compiler
python3 student.tests/pa6/build_checks.py /tmp/pa7-sanitize --sanitize
g++ -std=c++11 -O1 -g -fno-omit-frame-pointer -fsanitize=address,undefined -fno-pie -no-pie -Idev/src student.tests/pa7/check_api.cpp /tmp/pa7-sanitize/*.o -o /tmp/pa7-sanitize/pa7-api
/tmp/pa7-sanitize/pa7-api
python3 student.tests/pa7/check.py /tmp/pa7-sanitize/compiler
python3 student.tests/pa7/check_course.py /tmp/pa7-sanitize/compiler
```

The API checks selected conversions, separate incoming/outgoing call facts,
canonical specialization reuse and demand closure on the one retained graph.
The Python checks use independent snippets/rejections and verify separate TUs.
The alternate-binary runner reads course fixtures without modifying them.

`benchmark.py measure BASE FINAL record.json` freezes binary/input identities,
uses pinned-CPU A/A calibration and ABBA blocks, retains all observations and
measures compiler wall/RSS and host `.text`. `verify` checks the full protocol,
equivalent output and explicit budgets; `report record.json` renders paired
results and spread. Inherited workloads compare PA6 against PA7. New semantic
workloads use the same final binary for A and B (absolute cost and scaling, no
speedup claim). There is no generated executable at this stage.

The independent final audit adds 63 expression, conversion, constant, pointer,
cast, scope and demand probes. The fact API checks indirect callee preservation,
separate call-target/value identity, nested operator/call conversion ranges,
compound stores, variadic promotions and unevaluated member demand.

`benchmark.py measure BEFORE FINAL record.json --audit-delta` compares the six
fixed PA7 semantic workloads against a frozen pre-audit PA7 binary; `verify`
accepts the same option. The ordinary full corpus still compares PA6 versus PA7
for inherited modes and measures absolute PA7 cost/scaling. Budgets and input
generators are unchanged. Current records and any retained failed timing campaign
are identified in `pa7/performance.md`.

Timing follow-ups may select complete 1x/4x groups with `--prefix=...` and
increase independent translation units per sample with `--repeat-factor=N`.
Pass the same options to `verify`. These options are recorded and validated;
source bytes, ABBA ordering and numeric budgets remain fixed. `taskset -c 4`
selects CPU 4 before the harness pins itself. CPU user/system time and context
switches supplement wall time in the final campaigns; they do not replace the
wall-time acceptance criterion. Failed and superseded observations are retained.

The final full-corpus verification uses
`verify BASE FINAL audit-final-performance.json --recheck=audit-long-performance.json`
(with the full paths listed in `pa7/performance.md`). This validates all original
identity/output/work checks and the longer complete source group before
superseding only that group's timing. The exact original failure is printed;
no raw observations or other failing workload can be silently discarded.
Personal rejection probes require exit 1 and reject sanitizer diagnostics.
