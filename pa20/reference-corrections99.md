# Aggregate member-copy oracle correction

Pinned bundle revision `c2f713cd70d06170632bfde3e75dd6fe1aa44d98`, bundle
SHA256 `c532a109ae800825da24f60ae28ea894aa4896f56efb6ebb14728cdccf25a7d7`.
Local revision: **pa20-aggregate-member-copies-99**. Original oracle bytes are
preserved at entry `16ac49da1339edaa4a5f96bf4efab731a30c65e1`.

The required source returns `{1, first, second}` as an aggregate whose last
two members have nontrivial copy constructors. Both named parameters are
lvalues. N3337 §8.5.1 [dcl.init.aggr]/2 copy-initializes each member from its
corresponding clause; §8.5.4 [dcl.init.list]/4 sequences those clauses. Thus
each member is copy-constructed once from its corresponding source parameter,
in declaration order. The list is not an eligible named return object for
the implicit-move rules in §12.8 [class.copy]/32. Nothing in /31's permission
to omit certain copies permits introducing two new observable moves.

Sources: [aggregate member initialization](https://timsong-cpp.github.io/cppwp/n3337/dcl.init.aggr#2),
[list sequencing](https://timsong-cpp.github.io/cppwp/n3337/dcl.init.list#4),
[copy elision and implicit move](https://timsong-cpp.github.io/cppwp/n3337/class.copy#31).
The corresponding clauses are also in [the local standard draft](../doc/n3485.txt).
This establishes the required behavior independently of compiler agreement.

The old oracle constructs two temporary copies, then moves them into the
aggregate through a helper. The [reducer](../student.tests/pa20/member_copies99.cpp)
defines those moves, counts copies/moves and checks self-pointers and values.
The pinned reference executable fails its two-copies/zero-moves check; the
student executable passes. On the unchanged required fixture, the extra move
even causes an unresolved native symbol, because the source only declares it.

The [revision script](../student.tests/pa20/reference99.py) reads the original
oracle, preserves its copy-constructor body and main, and hand-constructs the
three ordered destination actions. It does not read student LowIR. The
ordinary visible `member_value(int)` definition is retained as a definition
root, with its exact two member initializers; it is not called by the return.
No move declaration or aggregate move helper remains. The script validates
the replacement with the supplied backend, executes it and the reducer, and
records original/replacement/source hashes in the revision manifest.

Only this `.ref` is revised. Source, success status, all 144 fixture identities,
comparison rules and member-copy/order/identity coverage remain intact. This
correction does not replace the separate implementation progress on omitted
class transport and class result boundaries.
