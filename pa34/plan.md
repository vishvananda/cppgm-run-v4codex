# PA34 inception

PA34 adds no feature or mode. Preserve the shared typed production pipeline,
all PA1–PA33 contracts, canonical generations and byte comparisons (spec §§1–10).
No additional performance gates: use stage-scoped acceptance, the handout's
900/3600-second compile limits and 8 GiB command RSS cap. Investigate divergent
self latency/RSS with frozen A/B, A/A and ABBA evidence before optimization.

## Remaining divergences and validation

- Fixed PA5 syntax/class_parser: elaborated class uses now reuse visible class
  bindings without hiding a callable name (`test_runner.cpp` sigaction call).
  `student.tests/pa5/check_elaborated.py`: six parser cases and runtime pass;
  C++11 [basic.lookup.elab] ¶1–3, [dcl.type.elab] ¶2. Explicit local forward
  declarations retain their scope. Original-source probe now compiles.
- Fixed PA29 builtin registry/semantic construction: `__builtin_trap` missing
  in retained queries for the hosted array header. It shares the typed abort
  operation permitted by the GNU contract; `student.tests/pa29/trap.md` records
  the proof. Probe/noexcept/arity and ordinary/template termination at O0/O3
  pass; the original PA2 source probe compiles. Host regression is rerunning.
- Continue each newly exposed self-build/test/object divergence at its earliest
  owner. Trace seed/self differences to object and source; probes are diagnostic.
- Required order: file audit; `make test-report-through-pa33`; canonical
  `make -C pa34 test-through-pa5 CXX=../dev/cppgm++ CPPGM_HOST_CXX=g++`;
  pptoken inception comparison; full `make inception CXX=g++ CPPGM_HOST_CXX=g++`.
  Continue the handout's self ladder through PA8/PA33 and audit source-to-ELF
  ownership, performance evidence and reproducibility before completion.
- Commit cohesive fixes and evidence, keep this ledger current, finish clean.

Current evidence (`$RALPH_ARTIFACT_DIR/pa34-221`): file audit passes (four
inherited warnings); host through PA33 5454/5454 after the parser fix. Canonical
self PA1 passes 54/54; PA2 stopped at the now-fixed trap query. A fresh host
regression followed by the canonical self ladder is running. Inception remains
unverified. Historical generated IR was gzip-compressed to reclaim 840 MiB;
its contents and measurement records are preserved.
