template<class T> struct Direct {
 __attribute__((exclude_from_explicit_instantiation)) int invalid() { return T::missing; }
};
template int Direct<int>::invalid();
