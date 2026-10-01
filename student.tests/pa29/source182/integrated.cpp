int made,dead,copied;
struct Z { int empty[0]; Z(){++made;} Z(const Z&){++copied;} ~Z(){++dead;} };
template<int N> struct F {
    __attribute__((always_inline)) static int operator()(int seed) {
        Z source[2];
        auto [a,b]=source;
        const int values[2]={seed,N};
        auto [x,y]=values;
        static_assert(__is_same(decltype(x),const int),"typed template array");
        if(seed<0)throw x+y;
        return x+y+sizeof(a)+sizeof(b);
    }
};
int main() {
    if(F<7>()(3)!=10 || made!=2 || copied!=2 || dead!=4)return 1;
    try { F<8>()(-3); return 2; } catch(int n) { if(n!=5)return 3; }
    return made!=4 || copied!=4 || dead!=8;
}
