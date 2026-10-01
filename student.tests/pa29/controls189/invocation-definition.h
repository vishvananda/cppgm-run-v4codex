template<class T> T&& demand_value() noexcept;
template<class F, class A> struct __is_nothrow_invocable;
// A real partial specialization overrides the source's false primary for
// lvalue function objects. It computes the answer from the call expression.
template<class F, class A> struct __is_nothrow_invocable<F&, A> {
  static constexpr bool value = noexcept(demand_value<F&>()(demand_value<A>()));
};
