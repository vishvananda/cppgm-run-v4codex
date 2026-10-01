extern "C" {
void ordinary_increment(_Atomic(int)* p){++*p;}
int increment(int* p){return __atomic_add_fetch(p,1,0);}
void bits(unsigned* p,unsigned value){__atomic_fetch_or(p,value,0);}
int acquire(const int* p){return __atomic_load_n(p,2);}
void release(int* p,int value){__atomic_store_n(p,value,3);}
struct Pair { unsigned long low,high; };
Pair get_pair(const Pair* p) { Pair r;__atomic_load(p,&r,2);return r; }
void set_pair(Pair* p,Pair r) { __atomic_store(p,&r,3); }
}
