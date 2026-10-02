struct Base { typedef int result; enum code { ok = 3 }; };
template<class T> struct Service : Base {
    result get(T) const;
    code status() const;
};
template<class U> typename Service<U>::result Service<U>::get(U) const { return 9; }
template<class U> typename Service<U>::code Service<U>::status() const { return Base::ok; }
int main() { Service<long> s; return s.get(4L) + s.status() - 12; }
