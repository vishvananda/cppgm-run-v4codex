struct Three {char a,b,c;};struct Twelve {int a,b,c;};
template<class T> struct Box { _Atomic(T) value; explicit Box(T v):value(v) {} };
static_assert(sizeof(Box<Three>)==4 && alignof(Box<Three>)==4,"three rounded");
static_assert(sizeof(Box<Twelve>)==16 && alignof(Box<Twelve>)==16,"twelve rounded");
Box<Three> global(Three{1,2,3});
int main(){
 Box<Three> x(Three{4,5,6});struct Guard {Three value;char sentinel;} out={{0,0,0},99};
 out.value=__c11_atomic_load(&x.value,2);if(out.value.a!=4 || out.value.c!=6 || out.sentinel!=99)return 1;
 Three expect={9,9,9};
 if(__c11_atomic_compare_exchange_strong(&x.value,&expect,Three{7,8,9},5,2) || expect.c!=6)return 2;
 if(!__c11_atomic_compare_exchange_weak(&x.value,&expect,Three{7,8,9},5,2))return 3;
 Three prior=__c11_atomic_exchange(&x.value,Three{10,11,12},4);if(prior.c!=9)return 4;
 __c11_atomic_store(&x.value,Three{13,14,15},3);out.value=__c11_atomic_load(&x.value,2);
 if(out.value.c!=15 || out.sentinel!=99)return 5;
 Box<Twelve> wide(Twelve{21,22,23});Twelve w=__c11_atomic_load(&wide.value,2);if(w.c!=23)return 6;
 if(!__c11_atomic_compare_exchange_strong(&wide.value,&w,Twelve{31,32,33},5,2))return 7;
 out.value=__c11_atomic_load(&global.value,2);return out.value.c==3 && out.sentinel==99?0:8;
}
