# PA27 implementation handoff

Stage base commit: `f833cf1ff361529147361cada33eca62e55330cf`
Last reviewed commit: `f833cf1ff361529147361cada33eca62e55330cf`
Target: **PA27 full-stage**. Phase: **implement; incomplete handoff147**.
Entry HEAD: `8d595af6b91289109cae36554a8e8e8551192873`, clean, **155/158**.
Current: **157/158**, failures **3 → 1**, no new failures or reduced coverage.
Earlier PAs: **4283/4283**. Previous goal turn: **progress**, with committed
ABI/emission implementation and evidence; no interrupted process was live.
Independent review markers above remain unchanged.

## Design/spec alignment and completed groups

| Owner | Data flow, complexity and boundary | Validation |
|---|---|---|
| Constructor actions | Canonical injected-field/storage edges select each immediate union variant. A declaration-order walk publishes shared parent paths and cumulative layout offsets. O(selected declarations + explicit initializer paths); one 24-byte path per selected storage, 28-byte action per initialized leaf/base. TU-owned vectors; no source cloning or name recovery. | Both formerly failing fixtures; nested fields, defaults, unions, refs, arrays/bit-fields, nontrivial copy/move |
| Default initialization | One declaration-owned context per class with demanded defaults; destination and `this` receiver paths remain separate. Scope/type identity is retained across constructor reuse. Anonymous structs receive ordinary default/destruction validation; union variant restrictions remain. | Mixed explicit/default constructors, prior-field reads, `this`, addresses, constructor-parameter rejection; PA11 enclosing constant |
| Runtime/lifetime lowering | Typed actions feed direct LowIR offsets for construction and partial unwind; ordinary destruction retains its storage boundary. No preliminary default construction or representation copy replaces projected nontrivial actions. O(actions + emitted IR); existing cleanup bounds remain. | O0/O2 constructor/member throws, lifetime counters, host execution and object controls |
| Constant construction | Canonical addresses index flat temporary group/part buffers. Freeze each selected storage once; include projected reference dependencies in activation keys. O(leaves + storage edges + required reachable dependencies), released with the activation; immutable values remain TU-owned. | Nested/union constexpr values, reads during initialization, missing-field rejection, reference arguments, measured constant workload |
| Declarators | Consume GNU attributes after pointer/reference/member-pointer operators alongside qualifiers, preserving ordinary attribute facts. Linear token work; no token replay. | 18 explicit controls; hosted fixture progresses beyond `<exception>` |

A nontrivial `S` and demanded `Value<N>` follow the same path: parsed declaration
→ canonical fields/layout → constructor-owned action/path slices → typed LowIR
construction and cleanup → PA24 selection/encoding → direct ELF. The focused
benchmark checks 600 demanded template classes, exactly 600 projected storage
paths and 1800 actions. No production phase adds a text roundtrip or host code
generation. Host linking remains the PA27 harness boundary.

Prior145/146 object placement, GOT, COMDAT/FDE ownership, ABI naming, semantic
linkage, TLS, suppression and support-entry work is preserved. Its designs and
measurements remain in [performance145](../student.tests/pa27/performance145.md),
[performance146](../student.tests/pa27/performance146.md) and their evidence.

## Unfinished implementation — not independent review

`200-host-extern-template-vtable-reference.t` remains failing. Pointer-attribute
parsing now succeeds; compilation reaches `<typeinfo>` and rejects
`__builtin_strcmp`. Hosted builtin semantics and subsequent library/extern-template
vtable demand are unfinished. The fixture, successful reference, comparison and
requirement are preserved; its vtable behavior has not yet been validated.

The anonymous-storage initialization/lifetime/constant group is complete at this
boundary, including defects found by extending beyond the two course failures.
The next failure is in hosted-library builtin binding before vtable demand,
which requires a separate semantic/runtime-helper contract and controls; changing
storage actions further cannot address it. This is an incomplete **assignment**
handoff, not certification of PA27 or permission to advance.

## Performance and validation

[Performance147](../student.tests/pa27/performance147.md) records frozen binaries,
flags/inputs, compiler latency/RSS, checked executable runtime/text, A/A noise and
six ABBA blocks: **448 final / 1232 retained observations**. It preserves
intermediate observations and uses pinned final measurements. No optional transform or new percentage gate was introduced.
Necessary scope/path costs and linear work/memory bounds are explicit.

[Validation147](../student.tests/pa27/evidence147/validation.json) pins the tested
implementation and binary. Prior-through, file audit (four inherited warnings),
and all **261 personal command checks** pass. PA27 is **157/158**, section controls
**3/3**; through-PA27 is **4440/4441**, with only the hosted fixture remaining.
All **19,749** tracked contract paths and **158** stage anchors are unchanged.
No fixture, reference, harness or comparison changes; the prior145 documented
[reference overlay](reference-corrections.md) is unchanged.

## Ledger and independent review

- Prior145: `c36f3501`, `96e1cb72`, handoff `5aefb962`.
- Prior146: `b0553a06`, `bb608e0b`, handoff `8d595af6`.
- Entry147: `7c71b6e6`, preserved stage/review markers and full-stage objective.
- `1c0b2d10`: storage paths, union/default/lifetime/constant actions and controls.
- `ebf263c4`: preserve enclosing-local constant folding in default contexts.
- `2eba9b7b`: pointer-attribute parser group and controls.
- `53b8a76d`, `79dffd03`: complete projected reference dependencies and flat
  temporary constant buffers; final required checks and performance use `79dffd03`.
- Handoff147 boundary: storage/default/lifetime/constant and pointer-attribute
  groups validated; hosted builtin/library/vtable behavior remains unfinished.
- Independent audit must examine default-context/receiver reuse, nested active
  variant and cleanup ownership, constexpr dependency completeness and recorded
  costs. Preserve prior145/146 questions about linkage-name/type identity,
  substitution slots, suppression versus demand, TLS hook/guard lifetimes,
  internal support-cache isolation, source attributes, COMDAT/FDE ownership and
  GOT scratch lifetimes. These are review questions, separate from the unfinished
  hosted implementation above; neither category is waived.
