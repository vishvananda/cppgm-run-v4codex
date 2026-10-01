__attribute__((always_inline)) inline int recurse(int n) {
    return n ? recurse(n-1)+1 : 0;
}
int main(int argc,char**) { return recurse(argc+3) != argc+3; }
