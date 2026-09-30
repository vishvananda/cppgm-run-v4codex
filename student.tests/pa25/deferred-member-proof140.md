# Disposition of the inherited unused-dependent-local reducer

The unchanged [reducer](unused-dependent-local.cc) names `Item<N>::missing`
inside the definition of `Item<N>`. `Item<N>` is the current instantiation;
there is no such member and there are no dependent bases. It is not a member
of an unknown specialization.

C++11 N3485 §14.6.2.1 [temp.dep.type]/5–6, in the
[checked-in standard](../../doc/n3485.txt), explicitly makes this ill-formed,
even without instantiating the containing template. Paragraph 6 contains the
matching example `typename A<T>::other j`. A diagnostic is permitted, though
not required. The rejection is therefore not evidence of premature member-body
instantiation, and the inherited plan's implementation item is closed by this
standard proof rather than by weakening a requirement.

The [valid deferred-body control](unused-dependent-member.cc) uses
`typename T::missing`, with `T` a template parameter instead of the current
instantiation. A valid definition can exist for some T. Instantiating only
`Item<int>::value` compiles and returns zero without demanding `unused`.
Both controls were run explicitly; the invalid reducer is rejected and the
valid control executes successfully. No course fixtures or reference outputs
changed. Independent audit should verify this disposition and demand ownership.
