struct S { char chars[5]; int data[3]; int tail; };
constexpr S s={.chars="abc",.data={2,4}};
static_assert(s.chars[3]==0 && s.chars[4]==0 && s.data[2]==0 && s.tail==0,"nested tails");
int main(){int v=(int){7};S x={.data{1,2,3}};return v!=7 || x.chars[0]!=0 || x.data[2]!=3 || ((int[3]){2,3,4})[1]!=3;}
