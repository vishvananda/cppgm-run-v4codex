// Reduced char_traits case: substitution cannot manufacture a nested type.
template<class T> struct traits;
template<class T> struct probe { typedef typename traits<T>::int_type type; };
probe<int>::type value;
