struct Base { typedef int result; };
template<class T> struct Service : Base { long get(T) const; };
template<class U> typename Service<U>::result Service<U>::get(U) const { return 9; }
int main() { return 0; }
