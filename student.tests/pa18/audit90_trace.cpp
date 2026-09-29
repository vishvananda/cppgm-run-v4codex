// One demanded specialization serves a direct call with a mapped default and
// a function-pointer call. The class copy and both destruction paths stay live.
int defaults, copies, dead;
int next() { return ++defaults; }
struct Packet {
    int value;
    Packet(int n) : value(n) {}
    Packet(const Packet& p) : value(p.value) { ++copies; }
    ~Packet() { ++dead; }
};
int sum(int a, long b) { return a+b; }
int sink(...) noexcept { return copies-dead; }
template<class... T> int dispatch(T... values, int bias=next()) {
    Packet p(sum(values...)+bias);
    static_assert(!noexcept(sink(p)), "copy may throw");
    return p.value+sink(p);
}
template<class T> using Identity = T;
struct Result { int value; Result(int n) : value(n) {} };
template<class T> Identity<T> make(int n) { return T(n); }
int main() {
    int (*call)(int,long,int)=dispatch<int,long>;
    Result (*construct)(int)=make<Result>;
    Result a=construct(dispatch<int,long>(6,7));
    Result b=construct(call(6,7,2));
    return a.value!=15 || b.value!=15 || defaults!=1 || copies!=2 || dead!=4;
}
