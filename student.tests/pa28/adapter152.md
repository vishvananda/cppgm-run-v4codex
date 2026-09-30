# PA28 native and course exception adapters

The course source-to-LowIR presentation and native production already use
distinct runtime ABI adapters (`Linkage::presentation`/`host`): exception payload
support storage, begin-catch signatures, pointer catch binding, and several
lifecycle conventions differ. Neither serializes/reparses between production
phases. This change keeps their existing handler conventions explicit.

Native handler-only expression regions use `EhCleanup` so their continuation
retains the expression frame until its explicit `EhEnd`. Native catch-exit
landings own the still-live outer lexical prefix. The course presentation uses
its established `EhTry` convention and separate call-unwind continuation for
that prefix. The common semantic lifetime/handler facts remain canonical.

A proposed reference correction was withdrawn: observation did not prove the
reference wrong. The reduced [region input](cleanup-region152.lowir) returns
zero under the pinned course backend both with `eh_try ^expression_exit` and
with `eh_cleanup ^expression_exit`. Its stack crosses an expression exit, an
active catch exit, and an outer handler. The source reducer also succeeds with
the reference host compiler. This confirms that our native region-retirement
assumption cannot justify changing the course presentation contract.

The original PA21 reference files are restored byte-for-byte; fixture sources,
statuses, coverage and comparison rules never changed. Both affected source
checks and the complete earlier suite pass with the original references.
`exceptions152.py` validates the corrected native host and private paths.

Reference bundle source: `c2f713cd70d06170632bfde3e75dd6fe1aa44d98`.
Raw observations, including unrelated duplicate-symbol failures when trying
to execute source presentation directly, are retained in evidence152. They
are observations, not correctness proofs or replacement implementations.
