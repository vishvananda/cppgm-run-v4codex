# PA21 inherited array contract and raw handler regions (112)

Bundle source `c2f713cd70d06170632bfde3e75dd6fe1aa44d98`, SHA256
`c532a109ae800825da24f60ae28ea894aa4896f56efb6ebb14728cdccf25a7d7`.
Local revision **pa21-array-contract-raw-handler-112** corrects two references.
[Reconstruction](../student.tests/pa21/reference112.py) applies exact replacements
to entry `9457e2fc`, without reading student output. The
[manifest](../student.tests/pa21/reference112-revision.json) retains both hashes
and every replacement. Sources, exit-status oracles, comparison rules, template
specializations and coverage are unchanged. The four earlier reference
revisions remain; `reference106.py` verifies its intermediate bytes and then the
112 continuation instead of discarding the 106 proof.

## Constant automatic arrays

The local-class-specialization fixture initializes `int[2]` and `long[2]` from
`{0,0}`. [PA16's Assignment Boundary](../pa16/README.md#assignment-boundary)
explicitly requires an automatic nonvolatile trivial scalar array with a fully
constant initializer to be initialized from readonly data with **one object
copy**, preserving distinct automatic storage. PA21 inherits this behavior from
PA20 and earlier milestones. The old reference's element stores contradict that
output contract. The 111 plan overlooked this explicit inherited rule when it
recorded the PA17/21 forms as an unresolved policy choice.

The correction adds two typed readonly images and replaces each array's stores
with one size/alignment-correct `copyobj`, retaining the original two stack
slots, every use, both local class identities and both destructor loops. This is
a **course output-contract correction**, not a claim that scalar stores violate
C++11. C++11 N3485 [dcl.init.aggr] 8.5.1/2 and [basic.stc.auto] 3.7.3/1
([local text](../doc/n3485.txt)) determine the initialized element values and
separate automatic lifetimes; LowIR's [object copy](../pa8/lowir.md#bulk-object-memory-operations)
operation preserves those values in the distinct destination objects.

The reduced `array_distinct_mixed`, `array_local_specializations` and
`array_in_handler` sources in [dispatch112.py](../student.tests/pa21/dispatch112.py)
mutate one copy, check the other, and compose the images with per-specialization
local destructors and source handlers. The inherited PA16 initialization
controls are also run explicitly. The execution driver below verifies **both**
reference forms produce status zero: semantics alone cannot justify choosing
one representation; the cited PA16 rule does. No frontend special case was added.

## Raw nested active-handler cleanup

The remaining `catch_cleanup_15` path in the nested-handler fixture is distinct
from the catch-miss path corrected in 106. Its entry retains these regions,
from outermost to innermost:

| Region | Installation | Required exit before the outer catch entry |
|---|---|---|
| Outer try | `eh_try ^catch_dispatch_1` | final `eh_end` |
| Outer active catch | `eh_cleanup ^catch_cleanup_9` | `eh_end` after ending that catch |
| Inner active catch | `eh_cleanup ^catch_cleanup_15` | `eh_end` after ending that catch |

The intervening inner try has already been retired before its catch becomes
active. Cleanup-style entry retains its own protected region; an
`eh_end_catch` **call** ends the caught exception's lifetime, not a LowIR region.
The old path supplies only two `eh_end` operations for these three remaining
regions. It also retains the completed inner catch's region across destruction
of the outer catch's `Guard`. The correction retires the inner region before
that outer cleanup, retains `Guard` destruction before ending the outer catch,
and explicitly retires the outer try before jumping to its catch entry.

The proof is LowIR's [explicit handler-stack operations](../pa8/lowir.md#handler-stack-management):
pushes have lexical identity, `eh_end` pops one region, and an ordinary jump is
not a region-pop operation. The intended source ordering follows C++11 N3485
[except.ctor] 15.2/1, [except.handle] 15.3/7–8 and [except.throw] 15.1/4
([text](../doc/n3485.txt:21421)). This correction preserves all destructor and
end-catch calls, their lifetime ordering, and the existing 106 changes.

[reference112_execution.py](../student.tests/pa21/reference112_execution.py)
records the limit of backend evidence explicitly. Making the previously dormant
pad reachable with an externally controlled throw still executes successfully
for **both** full reference versions: the supplied backend reconciles region
stacks at the shared outer catch entry. That tolerance is not the explicit
LowIR exit required by the contract. The IR-only reducer retains the three
region installations and gives their cleanup paths separate return terminals,
so a shared catch-entry join cannot conceal the missing pop. Two exits are
rejected with `protected region remains active`; three are accepted. The reducer
and full execution observations are both retained, including the unsuccessful
attempt to distinguish the full versions by execution alone.

The source implementation already emitted the explicit raw exits. Its separate
remaining mismatch was the synthetic miss edge after a catch-all. C++11
[except.handle] 15.3/5 proves that selector dispatch exhaustive. Lowering now
retains the O0 branch skeleton and uses a parent-linked dispatch fact to omit
exits only when forwarding to an enclosing catch-only entry (or resuming with
no parent). Cleanup-bearing joins and active-handler parents retain balanced
exits even on this impossible edge. Real typed-catch misses still carry their
complete live state. The initial broader omission broke the rethrow control;
[initial observations](../student.tests/pa21/dispatch112-initial.json) are
preserved, and the refined rule passes all 22 new controls.
This matches the original catch-all reference edge without revising it.
