int copies;
struct Base { int n; };
struct Value : Base {
    Value(int x) { n=x; }
    Value(Value const& x) : Base(x) { ++copies; }
    Value(Value&&) = default;
};
int take(Value v) { return v.n; }
struct Large { long first,second,third; };
Large forward(Large x) { x.second += 4; return x; }
struct Identity {
    Identity const* self;
    int value;
    Identity(int n):self(this),value(n){}
    Identity(Identity const& x):self(this),value(x.value){}
    ~Identity() { if(self!=this)__builtin_abort(); }
};
int identity(Identity x) { return x.self==&x ? x.value : -1; }
int main() {
    Value value(7);
    if(take(static_cast<Value&&>(value))!=7 || copies) return 1;
    Large x={1,2,3}; Large y=forward(x);
    if(y.first!=1 || y.second!=6 || y.third!=3 || x.second!=2) return 2;
    Identity a(9);
    return identity(a)!=9 || a.self!=&a;
}
