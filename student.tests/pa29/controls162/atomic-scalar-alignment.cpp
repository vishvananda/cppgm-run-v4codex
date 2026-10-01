typedef __int128 A __attribute__((aligned(1)));
struct Holder {char guard;A value;};alignas(16) Holder storage;
int main(){A* p=&storage.value;__atomic_store_n(p,9,5);
if(__atomic_load_n(p,5)!=9)return 1;
if(__atomic_exchange_n(p,12,5)!=9)return 2;
__int128 e=8;if(__atomic_compare_exchange_n(p,&e,4,false,5,2)||e!=12)return 3;
if(!__atomic_compare_exchange_n(p,&e,4,false,5,2))return 4;
if(__atomic_fetch_add(p,3,5)!=4 || __atomic_sub_fetch(p,2,5)!=5)return 5;
if(__atomic_fetch_xor(p,6,5)!=5 || __atomic_and_fetch(p,1,5)!=1)return 6;
if(__sync_val_compare_and_swap(p,1,7)!=1)return 7;
if(__sync_lock_test_and_set(p,2)!=7)return 8;__sync_lock_release(p);
return __atomic_load_n(p,5)==0?0:9;}
