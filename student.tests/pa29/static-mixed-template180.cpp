struct F {
    template<class T> static int operator()(T*) { return 1; }
    template<class T> int operator()(T) const { return 2; }
};
int main() { int x=0; return F()(&x)!=1 || F()(x)!=2; }
