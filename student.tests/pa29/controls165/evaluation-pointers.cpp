constexpr bool active(){return __builtin_is_constant_evaluated();}
int a=4,b=6;
int first(){return 3;} int second(){return 7;}
struct Base { int value; constexpr Base(int n):value(n){} };
struct Derived:Base {int tag;constexpr Derived():Base(active()?5:9),tag(active()?2:4){} };
union Variant {int n;char c;constexpr Variant():n(active()?21:25){} };
struct Bits {unsigned n:3;unsigned m:4;constexpr Bits():n(active()?3:5),m(active()?7:9){} };
int main(){
 constexpr int* p=active()?&a:&b;
 int* q=active()?&a:&b;
 constexpr auto f=active()?first:second;
 auto g=active()?first:second;
 constexpr Derived d; Derived r;
 constexpr Variant v; constexpr Bits bits;
 if(*p!=4||*q!=6||f()!=3||g()!=7)return 1;
 if(d.value!=5||d.tag!=2||r.value!=9||r.tag!=4)return 2;
 if(v.n!=21||bits.n!=3||bits.m!=7)return 3;
 return 0;
}
