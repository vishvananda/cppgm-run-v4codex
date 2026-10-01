template<class...T> using X=decltype((T()+...)); struct A{}; X<A,A> x;
