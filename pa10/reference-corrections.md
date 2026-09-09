# Mandatory constant initialization

Two sidecars from the pinned bundle incorrectly defer mandatory constant
initialization into `@__cppgm_init`. The source fixtures, exit statuses,
coverage and comparison rules are unchanged.

Bundle provenance: source `c2f713cd70d06170632bfde3e75dd6fe1aa44d98`, archive
`cppgm-reference-binaries-linux-x86_64-c2f713cd70d0.tar.gz`, SHA256
`c532a109ae800825da24f60ae28ea894aa4896f56efb6ebb14728cdccf25a7d7`;
see [manifest](../reference-binaries/manifest.tsv). These are local sidecar
corrections; the downloadable bundle itself has not been republished.

## Rule and proof

The course's [C++11 working draft](../doc/n3485.txt), §3.6.2
[basic.start.init]/2 (text lines 3836–3856), requires constant initialization
of a static-duration reference whose constant-expression initializer binds a
static-duration object, and of an object whose initializer consists entirely
of constant expressions. That initialization precedes every dynamic initializer.

§5.19 [expr.const]/2,4 (lines 7171–7274) defines the qualifying core, reference
and address constant expressions. `&values[1]` uses only a static-duration
array, integer literal, array-to-pointer conversion, valid pointer arithmetic,
indirection and address-of; none invokes a disallowed operation or reads the
array's elements. It is an address constant expression designating the second
static-duration subobject. `v` is a reference constant expression designating
the static-duration object. `x` also qualifies after its preceding constant
initialization; the reference-id exclusion explicitly allows that case.

Therefore:

- `200-global-array-element-address-initializer`: `middle` must have the address
  `values + 4` in static data, before dynamic initialization.
- `200-reference-chain-global-data`: `x` and `y` must both bind `v` during static
  initialization. LowIR represents those bindings as pointer relocations.

The minimal corrections replace the zero pointer data with those addresses
and remove the now-empty startup function. All other LowIR instructions and
objects remain. PA8's structured-data address/addend contract and PA10's
constant-array-element-address boundary directly support this representation.

## Observable reducers

[Address reducer](../student.tests/pa10/constant-address-order.cpp) defines an
ordinary dynamic initializer before the definition of `middle`. Its call must
observe the already constant-initialized pointer. [Reference reducer](../student.tests/pa10/constant-reference-order.cpp)
does the same for a reference binding. Both must return 0. Both pinned-reference
outputs instead return 1 through `lowir2native-ref -O0`: their startup functions
call the observer before storing the required address. This is an observable
ordering defect, not a preference for a smaller IR or host-compiler agreement.

Reproduce explicitly:

```sh
python3 student.tests/pa10/check_reference_corrections.py
```

The reducers intentionally include dynamic global calls to expose ordering;
that broader input lies beyond PA10's required constant-global subset. They
observe the pinned reference only. The corrected course fixtures still use
PA10's supported constant-global subset and are compiled by our implementation.
