template<class T> struct Outer {
    struct Inner {
        friend class Outer<T*>;
    private:
        int value;
    };
    int get(const Inner& p) { return p.value; }
};
int main() { return 0; }
