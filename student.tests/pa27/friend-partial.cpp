class Secret {
    int value=23;
    template<class T> friend struct Reader;
};
template<class T> struct Reader { static int read(const Secret& s) { return s.value; } };
template<class T> struct Reader<T*> {
    struct Nested { static int read(const Secret& s) { return s.value+1; } };
    static int read(const Secret& s) { int value=s.value; return value+Nested::read(s); }
};
template<> struct Reader<int> { static int read(const Secret& s) { return s.value+2; } };
int main() { Secret s; return Reader<long>::read(s)+Reader<char*>::read(s)+Reader<int>::read(s)-95; }
