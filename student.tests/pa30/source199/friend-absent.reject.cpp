template<class T> struct Outer {
    struct Inner { private: int value; };
    int get(const Inner& p) { return p.value; }
};
int main() { return 0; }
