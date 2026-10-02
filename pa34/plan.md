# PA34 inception

PA34 adds no language feature or mode. Preserve the shared typed pipeline,
PA1–PA33 contracts, canonical generations and exact object/binary comparisons
(spec §§1–10). [Corrections and reducers](fixes.md); [ownership audit](audit.md).

## Remaining work

- Resolve the self-only stack exhaustion compiling semantic/output.cpp. PA10
  lowering/expression carries an 18.7 KiB dispatcher frame at each nested call.
  With diagnostic 64 MiB stack, frozen A/A + six ABBA blocks show identical
  objects/work, 46,365-byte text, seed/self median 3.069/6.093 s and peak RSS
  262,976/279,476 KiB; paired ratio 1.972, range 1.424–2.005. This is code-quality
  evidence, not repeated lowering. Test a thin call dispatcher under the original
  8 MiB stack, preserving guards, invocation scopes and exact IR (spec §§6,8,9).
  Diagnostic dispatcher now uses 432 bytes and passes the original source plus
  400 nested calls. Before/after ABBA latency 1.006 [0.881–1.317], RSS drops
  4,268 KiB, identical generated text/work; canonical acceptance follows.
- Run frozen seed/self A/A + ABBA compiler and generated-runtime measurements,
  including RSS, text, exact outputs and work counters (spec §9). The inherited
  fixed template, memory, floating, EH and compiler-component inputs remain.
- Bind canonical results and evidence, commit the final audit and leave clean.

## Acceptance

No new performance ratio gate. Preserve the handout's 900/3600-second command
limits, 8 GiB RSS cap and existing optimization work/growth budgets. Investigate
self-only resource divergence as correctness evidence before changing valid
source constructs. Probe objects/links are diagnostic; final builds are canonical.

Required order: file audit; host through PA33; self through PA5 (then handout
PA8/PA33 ladder); pptoken inception; `make inception CXX=g++ CPPGM_HOST_CXX=g++`.

Current evidence (`$RALPH_ARTIFACT_DIR/pa34-221`): latest file audit and host
5454/5454, canonical PA5/PA8/PA33 and pptoken inception pass after the complex
order correction. Full inception next exposed the stack issue above. Native debug 11/11 and self
native driver 18/18 pass. Latest O0–O3 source/template trace matches LowIR, ELF
and work counters with 24 passing runtime checks. Original crashes, object/IR
mismatches, diagnostic binaries and reducer failures remain archived. Earlier
IR was losslessly gzip-compressed to reclaim 840 MiB; no measurements were lost.
