// N3485 [dcl.align]: alignment belongs to each concrete type. Deferred body
// demand must project attribute operands using the same specialization context.
template<class T> struct Owner {
    int run() {
        struct alignas(T) Local { char value; };
        Local object;
        return sizeof(object)==__alignof(T) && __alignof(Local)==__alignof(T);
    }
};
#pragma pack(push,1)
template<class T> struct Packed {
    char tag;
    T value;
    int run() {
        struct Local { char tag; T value; };
        return sizeof(Local)==1+sizeof(T);
    }
};
#pragma pack(pop)
int main() {
    Owner<int> a; Owner<long double> b;
    Packed<int> c; Packed<long> d;
    if (!a.run() || !b.run() || !a.run()) return 1;
    return sizeof(c)!=1+sizeof(int) || sizeof(d)!=1+sizeof(long) || !c.run() || !d.run();
}
