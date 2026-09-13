// Current-instantiation and raw alias parameters keep all declaring heads.
int live;
template<class T> struct Box {
    typedef T type;
    type value;
    Box(const type&);
    Box(const Box&);
    Box(Box&&);
    Box& operator=(const Box&);
    Box& operator=(Box&&);
    ~Box();
};
template<class U> Box<U>::Box(const type& x) : value(x) { ++live; }
template<class U> Box<U>::Box(const Box& x) : value(x.value) { ++live; }
template<class U> Box<U>::Box(Box&& x) : value(x.value) { x.value=0; ++live; }
template<class U> Box<U>& Box<U>::operator=(const Box& x) { value=x.value; return *this; }
template<class U> Box<U>& Box<U>::operator=(Box&& x) { value=x.value; x.value=0; return *this; }
template<class U> Box<U>::~Box() { --live; }
template<class T> struct Outer { struct Node; };
template<class U> struct Outer<U>::Node {
    typedef U type;
    type value;
    Node(const type&);
    Node(const Node&);
    Node& assign(const type&);
};
template<class V> Outer<V>::Node::Node(const type& x) : value(x) {}
template<class V> Outer<V>::Node::Node(const Node& x) : value(x.value) {}
template<class V> typename Outer<V>::Node& Outer<V>::Node::assign(const type& x) { value=x; return *this; }
template<class T> struct Defaulted {
    T value;
    Defaulted();
    Defaulted(const Defaulted&);
    Defaulted& operator=(const Defaulted&);
    ~Defaulted();
};
template<class U> Defaulted<U>::Defaulted() = default;
template<class U> Defaulted<U>::Defaulted(const Defaulted&) = default;
template<class U> Defaulted<U>& Defaulted<U>::operator=(const Defaulted&) = default;
template<class U> Defaulted<U>::~Defaulted() = default;
int main() {
    {
        Box<int> a(3); Box<int> b(a); Box<int> c(static_cast<Box<int>&&>(a));
        Box<long> d(7); Box<long> e(d); Box<long> f(static_cast<Box<long>&&>(d));
        if (a.value || d.value || c.value!=3 || f.value!=7 || live!=6) return 1;
        a=b; d=e; c=static_cast<Box<int>&&>(b); f=static_cast<Box<long>&&>(e);
        if (b.value || e.value || a.value!=3 || d.value!=7 || c.value!=3 || f.value!=7) return 2;
        Outer<int>::Node n(5); Outer<int>::Node m(n); Outer<long>::Node q(8);
        if (&m.assign(9)!=&m || m.value!=9 || n.value!=5 || q.assign(10).value!=10) return 3;
        Defaulted<int> first; first.value=11; Defaulted<int> second(first); first=second;
        if (first.value!=11 || second.value!=11) return 4;
    }
    return live!=0;
}
