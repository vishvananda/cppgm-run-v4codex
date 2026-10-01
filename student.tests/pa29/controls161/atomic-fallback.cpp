struct Triple { int a,b,c; };
struct Wide { unsigned long a,b; };
struct Big { unsigned long a,b,c; };
template<class T> struct Box {_Atomic(T) value;explicit Box(T v):value(v){}};
int main(){
 Triple x={1,2,3},out={},expect={1,2,3},want={4,5,6};
 __atomic_load(&x,&out,2);if(out.c!=3)return 1;
 if(!__atomic_compare_exchange(&x,&expect,&want,false,5,2) || x.c!=6)return 2;
 expect.a=9;if(__atomic_compare_exchange(&x,&expect,&out,false,5,2) || expect.a!=4 || expect.c!=6)return 3;
 __atomic_exchange(&x,&out,&expect,4);if(expect.a!=4 || x.a!=1)return 4;
 struct Holder { unsigned long before;Wide value; } holder={0,{11,22}};
 Wide value={33,44},r={};__atomic_store(&holder.value,&value,3);__atomic_load(&holder.value,&r,2);
 if(r.a!=33 || r.b!=44)return 5;
 Box<Big> big(Big{1,2,3});Big got=__c11_atomic_load(&big.value,2);if(got.c!=3)return 6;
 Big old=__c11_atomic_exchange(&big.value,Big{4,5,6},4);if(old.c!=3)return 7;
 if(__c11_atomic_compare_exchange_strong(&big.value,&got,Big{7,8,9},5,2) || got.c!=6)return 8;
 if(!__c11_atomic_compare_exchange_strong(&big.value,&got,Big{7,8,9},5,2))return 9;
 __c11_atomic_store(&big.value,Big{10,11,12},3);got=__c11_atomic_load(&big.value,2);if(got.c!=12)return 10;
 _Atomic(Big) init;__c11_atomic_init(&init,Big{21,22,23});got=__c11_atomic_load(&init,2);
 return got.c==23 && !__c11_atomic_is_lock_free(sizeof(Big))?0:11;
}
