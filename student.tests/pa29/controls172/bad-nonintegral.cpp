template<class...T> struct X{static_assert((__is_integral(T)&&...),"bad");}; X<int,double> x;
