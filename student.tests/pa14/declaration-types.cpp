// N3485 [dcl.type.simple], [dcl.typedef], [temp.dep.type], [temp.mem].
// Shared type facts must keep declaration heads and concrete environments apart.
template<class T> T locals(T value) {
    typedef T Value;
    using ConstValue = const Value;
    Value* pointer=&value;
    ConstValue* read=pointer;
    using Reference=decltype((value));
    Reference reference=*pointer;
    reference+=1;
    { Value Value=value, *other=&Value; if (*other!=*read) return 0; }
    const unsigned count=sizeof(T);
    T array[count];
    array[0]=*read;
    return array[0];
}
template<class T> struct Box {
    using Value=T;
    using Self=Box;
    T value;
    static const unsigned count=sizeof(T);
    Value get() const;
    static Value echo(Value);
    struct Inner {
        using Copy=T;
        Copy value;
        Copy get() const { Copy result=value; return result; }
    };
    int identity() { Self* p=this; return p==this; }
};
template<class U> typename Box<U>::Value Box<U>::get() const {
    Value copy=value;
    using Reference=decltype((copy));
    Reference ref=copy;
    return ref;
}
template<class V> typename Box<V>::Value Box<V>::echo(Value value) {
    const Value result=value;
    return result;
}
template<class T> struct Provider { typedef T value_type; };
template<class T> struct Derived:Provider<T> {
    using typename Provider<T>::value_type;
    value_type value;
    value_type get() { return value; }
};
int increment(int n) { return n+1; }
template<class T> struct FunctionField {
    T padding;
    int (*fn)(const int);
    int run(int n) const { return fn(n); }
};
template<class T> int function_local(T) {
    int (*fn)(const int)=increment;
    return fn(7);
}
template<class T> struct Inspector;
class Secret {
    int value;
    friend struct Inspector<Secret>;
public:
    Secret(int n):value(n) {}
};
template<class T> struct Inspector {
    int read(T& object) {
        using Value=decltype(object.value);
        Value copy=object.value;
        return copy;
    }
};
template<class T> struct QueryMembers {
    T value;
    mutable int edits;
    int& reference;
    QueryMembers(T n, int& r):value(n),edits(0),reference(r) {}
private:
    T& pick(int) { return value; }
    const T& pick(int) const { return value; }
    long pick(long) const { return 91; }
public:
    int read() const {
        using ValueRef=decltype((value));
        using EditRef=decltype((edits));
        using StoredRef=decltype((reference));
        ValueRef v=value; EditRef e=edits; StoredRef r=reference;
        ++e; ++r;
        return v;
    }
    int call() const {
        using Result=decltype(pick(1));
        Result result=pick(1);
        return result;
    }
};
int main() {
    if (locals(3)!=4 || locals(8L)!=9) return 1;
    Box<int> a; Box<long> b; a.value=5; b.value=9;
    if (a.get()!=5 || b.get()!=9 || !a.identity() || Box<long>::echo(13)!=13) return 2;
    if (Box<int>::count!=sizeof(int) || Box<long>::count!=sizeof(long)) return 3;
    Box<int>::Inner ia; Box<long>::Inner ib; ia.value=4; ib.value=11;
    if (ia.get()!=4 || ib.get()!=11 || sizeof(ib.value)!=sizeof(long)) return 4;
    Derived<int> da; Derived<long> db; da.value=6; db.value=12;
    if (da.get()!=6 || db.get()!=12) return 5;
    FunctionField<char> fc; FunctionField<long> fl; fc.fn=increment; fl.fn=increment;
    if (fc.run(2)!=3 || fl.run(6)!=7 || function_local(3)!=8) return 6;
    Secret secret(17); Inspector<Secret> inspector;
    if (inspector.read(secret)!=17) return 7;
    int referred=4;
    QueryMembers<int> qi(18,referred); QueryMembers<long> ql(23,referred);
    if (qi.read()!=18 || ql.read()!=23 || referred!=6 || qi.edits!=1 || ql.edits!=1) return 8;
    return qi.call()!=18 || ql.call()!=23;
}
