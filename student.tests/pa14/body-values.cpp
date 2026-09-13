// N3485 [temp.dep.expr], [temp.dep.constexpr]: sizeof/alignof have fixed
// size_t types while their values can depend on a template argument.
int observed=0;
int consume(unsigned long n) { ++observed; return n+3; }
template<class T> int compute(int x) {
    x+=sizeof(T);
    x*=sizeof(T)+1;
    x-=__alignof(T);
    if (sizeof(T)>1) x+=consume(sizeof(T));
    return x;
}
template<class T> int constant() {
    return (sizeof(T)+1)*2-__alignof(T);
}
template<class T> int nested() {
    return sizeof(sizeof(T))+sizeof(T[2]);
}
template<class T> struct Owner {
    int value;
    Owner(int n):value(n) {}
    int run(int x) const { return value+x+sizeof(T)+__alignof(T); }
    int outside(int);
};
template<class U> int Owner<U>::outside(int x) { return value+x+sizeof(U); }
int main() {
    if (compute<int>(2)!=(2+sizeof(int))*(sizeof(int)+1)-__alignof(int)+sizeof(int)+3) return 1;
    if (compute<long>(3)!=(3+sizeof(long))*(sizeof(long)+1)-__alignof(long)+sizeof(long)+3) return 2;
    if (observed!=2 || compute<char>(4)!=9 || observed!=2) return 3;
    if (constant<int>()!=6 || constant<long>()!=10 || constant<char[3]>()!=7) return 4;
    if (nested<int>()!=sizeof(unsigned long)+2*sizeof(int) || nested<long>()!=sizeof(unsigned long)+2*sizeof(long)) return 5;
    Owner<int> a(4); Owner<long> b(6);
    return a.run(3)!=15 || b.run(2)!=24 || a.outside(3)!=11 || b.outside(2)!=16;
}
