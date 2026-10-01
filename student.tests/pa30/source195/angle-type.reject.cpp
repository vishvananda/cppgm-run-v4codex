template<class T> struct wrap {};
template<class T> struct type_only {};
type_only<wrap<wrap<int>>{}> invalid;
