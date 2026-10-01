typedef int I __attribute__((aligned(1))); I i; static_assert(__alignof__(i)==1,"i"); static_assert(__alignof__(*(&i))==1,"indirect"); int main(){return 0;}
