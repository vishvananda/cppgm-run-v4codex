constexpr bool active(){return __builtin_is_constant_evaluated();}
constexpr int choose(bool b=active()){return b?5:7;}
template<int N=(active()?11:13)> constexpr int templated(){return active()?N:N+2;}
struct Box {int n; constexpr Box(bool b=active()):n(b?3:9){} };
static_assert(choose()==5&&templated()==11,"");
constexpr Box global;
int main(){
 const int k=choose(); int x=choose();
 constexpr Box cb; Box rb;
 int values[]={choose(),templated()};
 if(k!=5||x!=7||cb.n!=3||rb.n!=9||global.n!=3)return 1;
 if(values[0]!=7||values[1]!=13)return 2;
 const auto action=[](){return active();};
 if(action())return 3;
 return 0;
}
