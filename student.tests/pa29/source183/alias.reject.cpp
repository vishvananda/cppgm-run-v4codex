template<class T> struct B {};template<class T> using box=B<T>;box(int)->box<int>;
