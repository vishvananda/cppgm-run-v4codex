struct Pair { int a,b; };
int order_count=0;
int order(){++order_count;return 0;}
int main(){
 Pair value={1,2},desired={3,4},out={},expected={9,9};
 __atomic_load(&value,&out,order());if(out.a!=1 || out.b!=2)return 1;
 __atomic_store(&value,&desired,order());if(value.a!=3 || value.b!=4)return 2;
 desired.a=5;__atomic_exchange(&value,&desired,&out,order());if(out.a!=3 || value.a!=5)return 3;
 if(__atomic_compare_exchange(&value,&expected,&out,false,5,2) || expected.a!=5 || expected.b!=4)return 4;
 if(!__atomic_compare_exchange(&value,&expected,&out,true,5,2) || value.a!=3)return 5;
 volatile int x=0; __sync_fetch_and_add(&x,3);__sync_lock_release(&x);if(x)return 6;
 unsigned char small=255;if(__atomic_add_fetch(&small,2,0)!=1)return 7;
 unsigned __int128 wide=(unsigned __int128)1<<90;
 if(__atomic_fetch_add(&wide,5,0)!=((unsigned __int128)1<<90))return 8;
 unsigned __int128 want=wide;if(!__atomic_compare_exchange_n(&wide,&want,(unsigned __int128)3,false,5,2) || wide!=3)return 9;
 return order_count==3?0:10;
}
