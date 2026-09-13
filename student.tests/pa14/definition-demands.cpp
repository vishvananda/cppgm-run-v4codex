// Completed definition tails retain their specialization and source-head keys.
// A later overload or previously absent definition must still be applied.
template<class T> struct Methods {
    T value;
    static T shared;
    int read(int) const;
    int read(double) const;
    int read(char) const;
    int unused(float) const;
    struct Nested { int read(int) const; };
};
template<class T> int Methods<T>::read(int n) const { return value+n; }
int first() {
    Methods<int> a={5};
    return a.read(1)+a.read(2)+a.read(3);
}
int* early_address() { return &Methods<int>::shared; }
template<class T> int Methods<T>::read(double n) const { return value+int(n)+10; }
int second() {
    Methods<int> a={5}; Methods<long> b={7};
    return a.read(1)+a.read(2.0)+b.read(1)+b.read(2.0);
}
int before_char_definition() {
    Methods<int> a={9};
    return a.read(char(1));
}
template<class T> T Methods<T>::shared=T(23);
template<class T> int Methods<T>::read(char n) const { return value+n+20; }
template<class T> int Methods<T>::Nested::read(int n) const { return sizeof(T)+n; }
template<class T> int Methods<T>::unused(float) const { return T::missing; }
int main() {
    Methods<int>::Nested a; Methods<long>::Nested b;
    if (first()!=21 || second()!=50 || before_char_definition()!=30) return 1;
    if (*early_address()!=23 || static_cast<const void*>(&Methods<int>::shared)==static_cast<const void*>(&Methods<long>::shared)) return 2;
    return a.read(3)!=7 || b.read(3)!=11 || Methods<long>::shared!=23;
}
