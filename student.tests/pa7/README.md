# PA7 personal validation

Run explicitly from the root:

```sh
python3 student.tests/pa7/check.py
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
