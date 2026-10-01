struct alignas(16) Pair {long a,b;};
using Packed __attribute__((aligned(1))) = Pair;
struct Holder {char guard;Packed value;};
alignas(16) Holder memory;
Packed* address(){return &memory.value;}
template<class Tag> int operate(Packed* p){
 Pair a={4,7},e={4,7},b={8,9},r={};
 __atomic_store(p,&a,5);
 if(!__atomic_compare_exchange(p,&e,&b,false,5,2))return 1;
 __atomic_exchange(p,&a,&r,5);
 if(r.a!=8 || r.b!=9)return 2;
 __atomic_load(address(),&r,5);
 return r.a==4 && r.b==7?0:3;
}
int main(){return operate<int>(address())+operate<char>(address());}
