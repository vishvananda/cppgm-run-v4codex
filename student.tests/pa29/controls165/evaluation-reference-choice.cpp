constexpr bool active(){return __builtin_is_constant_evaluated();}
int x=4,y=7;
int main(){
 int a=3,b=5;
 const int& r=active()?a:b;
 const int& g=active()?x:y;
 const int& ar=(active()?a:b)+1;
 if(&r!=&a||&g!=&x)return 1;
 a=6;x=8;
 if(r!=6||g!=8||ar!=6)return 2;
 return 0;
}
