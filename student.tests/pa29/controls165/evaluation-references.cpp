constexpr bool active(){return __builtin_is_constant_evaluated();}
constexpr int mode(){return active()?3:7;}
struct Box {int value; constexpr Box():value(mode()){} };
int main(){
 const bool& a=active(); bool&& b=active();
 const int& c=mode(); const Box& box=Box();
 const double d=active()?1.:2.;
 if(!a||!b||c!=3||box.value!=3||d!=2.)return 1;
 b=false; return b?2:0;
}
