int main(){
 auto f=[]<class T=int>(T x=T(4)) noexcept(sizeof(T)<=sizeof(int)) {return x;};
 if(f()!=4 || f(7L)!=7) return 1;
 static_assert(noexcept(f()),"int call");
 static_assert(!noexcept(f(7L)),"long call");
 auto z=[]<int N=3>{return N;};
 if(z()!=3 || z.operator()<5>()!=5) return 2;
 return 0;
}
