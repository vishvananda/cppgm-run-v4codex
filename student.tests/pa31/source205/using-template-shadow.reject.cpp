struct B { static int T; };
template<class T> struct D : B { using B::T; };
D<int> d;
