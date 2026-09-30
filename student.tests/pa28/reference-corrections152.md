# PA21 cleanup-region reference corrections during PA28

Bundle source revision: `c2f713cd70d06170632bfde3e75dd6fe1aa44d98`;
bundle SHA-256: `c532a109ae800825da24f60ae28ea894aa4896f56efb6ebb14728cdccf25a7d7`.
The manifest/binaries remain pinned. Two checked LowIR outputs are corrected;
source inputs, statuses, coverage and comparison rules are unchanged.

## Proof and reducer

[PA8 LowIR Handler Stack Management](../../pa8/lowir.md#handler-stack-management)
distinguishes catch-style `eh_try` from cleanup-style `eh_cleanup`; `eh_end`
pops an installed region. [PA21 cleanup requirements](../../pa21/README.md#required-implementation-surface)
require handler exit, still-live outer objects, and identical complete region
stacks for shared cleanup continuations. N3485 [except.ctor]/1 and
[except.handle]/6–8 require automatic-object destruction on propagation and
ending the active handler before continuing the surrounding search.

In `400-handler-context-cleanup-continuation`, both `call_unwind_dispatch_20`
and `call_unwind_dispatch_23` reach a suffix that explicitly ends the expression
region and then the active catch region. Their old `eh_try` registrations carry
no catch/cleanup clauses and cannot describe the required retained cleanup
region. Changing precisely those two registrations to `eh_cleanup` supplies
that fact; instruction order and destruction/handler operations remain intact.

[cleanup-reducer152.cpp](cleanup-reducer152.cpp) makes this path observable:
`read(Value(7))` throws 1; its temporary is destroyed; construction of the
catch's `Value(11)` throws 2. Propagation must finish that catch and reach main's
handler with trace 7. There is no fully constructed Value(11) to destroy.
The old host pipeline rejects the region underflow; the corrected host pipeline
compiles, links and returns zero. The proof is the region/lifetime contract,
not compiler agreement. The reducer also exercises the private native pipeline.

In `200-source-handler-branch-call-cleans-outer-scope`, the same correction is
needed at `call_unwind_dispatch_13`. Its typed-catch miss also owns a live Guard:
add a cleanup clause and its matching region exit to `catch_dispatch_1`.
Finally `catch_cleanup_6` must destroy that Guard before resuming; previously it
omitted this live outer local. These are required by the handout's explicit
outer-scope cleanup rule, not alternative allocation or ordering preferences.
The existing input itself is a reducer for the branch/catch continuation;
[exceptions152.cpp](exceptions152.cpp) adds nested rethrows and ordered guards.

The fixes are four inserted LowIR instructions and three registration-opcode
changes. No reference output was replaced wholesale from a compiler dump.
