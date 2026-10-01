using I __attribute__((aligned(1))) = int; I i; static_assert(__alignof__(i)==1,"alias"); int main(){return 0;}
