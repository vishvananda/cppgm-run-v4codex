template<class P> typename P::type read(P);
template<class Q> typename Q::type read(Q p) { return p.value; }
template<class T> struct Traits { typedef T type; };
template<class T> typename Traits<T>::type echo(T value) { return value; }
struct Short { typedef short type; short value; };
struct Long { typedef long type; long value; };
template<class T> typename T::type checked(typename T::type* p,T) { return *p; }
int main() {
    Short s={7}; Long l={11};
    if (read(s)!=7 || read(l)!=11 || echo(13L)!=13) return 1;
    if (sizeof(read(s))!=sizeof(short) || sizeof(read(l))!=sizeof(long)) return 2;
    return checked(&s.value,s)!=7;
}
