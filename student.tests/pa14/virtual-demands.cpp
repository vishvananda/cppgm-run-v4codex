// Lifecycle recursion and virtual slot identities survive template/local-class growth.
int destroyed = 0;
struct Base { virtual int value() = 0; virtual ~Base() { ++destroyed; } };
template<class T> struct Box : Base {
    T data;
    Box(T x) : data(x) {}
    int value();
    ~Box();
};
template<class T> int Box<T>::value() {
    struct Local { virtual T twice(T x) { return x+x; } };
    Local local;
    return local.twice(data);
}
template<class T> Box<T>::~Box() { destroyed += 2; }
struct First { virtual int value(); };
struct Second { virtual int value(); };
int Second::value() { return 3; }
int First::value() { return 5; }
int invoke(Base& b) { return b.value(); }
int main() {
    {
        Box<int> a(7); Box<long> b(11);
        First first; Second second;
        if (invoke(a) != 14 || invoke(b) != 22 || first.value()+second.value() != 8) return 1;
    }
    return destroyed != 6;
}
