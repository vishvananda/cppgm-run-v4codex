# Implementation192 — extended floating formats

Entry HEAD: `4becf437ada14c36b01baae1d608428a621a5317`, clean, 399/403.
Stage base and Last reviewed commit are preserved in plan.md. The interrupted
entry left no live compiler/check process; current inspection establishes the
next action. This turn starts with three floating-format failures and one
independent nested-template ABI-tag contract question.

## Ownership, data flow, complexity and validation plan

The floating representation owner spans posttoken literal decoding, canonical
fundamental types, semantic constants/conversions, typed LowIR values and native
ABI/encoding. Binary16 and binary128 must retain distinct bits, precision,
size/alignment, overload identity and call/return classification; neither is an
alias for an existing format. Supported width aliases keep their actual host
format. Parsed type/literal facts feed semantic facts once; lowering consumes
them directly. LowIR text is an explicit inspection adapter only.

Implement the related group together: spellings/functional casts and suffixes;
exact constants and conversions; static and runtime storage; arithmetic,
comparisons and ABI; explicit LowIR roundtrips. Test precision beyond binary80,
subnormals, signed zero, aggregate/layout and cross-compiler calls. Own helper
logic and typed records in dev/src and register new sources. Per-operation
numeric work is bounded by the fixed formats; input decoding scales with input
length. No optimization pass or unbounded cache is planned.

Freeze entry compiler and benchmark inputs before changes. Measure latency/RSS
and runtime/text separately, checking results; use A/A and ABBA on equivalent
correct common workloads and corrected-only measurements for newly implemented
behavior. Keep raw observations and distinguish necessary semantic cost from
optional optimization. Preserve inherited measurements and mandated budgets.

After the floating group, investigate the remaining ABI-tag contract with its
existing reducer and explicit specification proof. Do not weaken the oracle or
encode a source-pattern exception. Finish related work while the same owner
understanding applies. Validate personal controls explicitly, make test-pa29,
PA1–28/root-through, and file audit; commit coherent increments and record the
actual remaining implementation and independent review questions separately.
