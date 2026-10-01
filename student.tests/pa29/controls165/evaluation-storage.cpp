constexpr bool active(){return __builtin_is_constant_evaluated();}
constexpr int choose(int x){return active()?x+4:x-2;}
constexpr int recurse(int x){return x?recurse(x-1)+1:(active()?10:20);}
constexpr int forced=choose(1);
int global=choose(1);
int runtime(int x){return x;}
int dynamic=active()?runtime(1):runtime(2);
struct Box { int value; constexpr Box():value(active()?1:2){} };
constexpr Box static_box;
int main(int argc,char**){
 bool ordinary=active(); const bool fixed=active(); constexpr bool required=active();
 volatile const bool observed=active();
 bool a[]={active(),choose(4)==8};
 const bool b[]={active(),choose(4)==8};
 constexpr bool c[]={active(),choose(4)==8};
 constexpr Box box;
 Box runtime_box;
 static int local=choose(1);
 constexpr int memo=recurse(4);
 if(ordinary||!fixed||!required||observed)return 1;
 if(a[0]||a[1]||b[0]||b[1]||!c[0]||!c[1])return 2;
 if(box.value!=1 || runtime_box.value!=2 || static_box.value!=1)return 3;
 if(forced!=5 || global!=5 || dynamic!=2 || local!=5)return 4;
 if(memo!=14 || recurse(4)!=24 || choose(argc)!=-1)return 5;
 return 0;
}
