template<class T> struct Outer {
    struct Inner {
        friend class Outer<T>;
    private:
        int value;
    public:
        Inner(int v): value(v) {}
    };
    int get(const Inner& p) { return p.value; }
};
template<class T> struct Simple {
    struct Inner {
        friend Simple<T>;
    private:
        int value;
    public:
        Inner(int v): value(v) {}
    };
    int get(const Inner& p) { return p.value; }
};
int main() {
    Outer<int> o; Outer<int>::Inner i(21);
    Simple<long> s; Simple<long>::Inner j(22);
    return o.get(i) + s.get(j) - 43;
}
