constexpr bool active(){return __builtin_is_constant_evaluated();}
constexpr bool fixed_local(){const bool x=active(); return x;}
constexpr bool required_local(){constexpr bool x=active(); return x;}
constexpr int dependent_local(int n){const int x=active()?n:17; return x;}
constexpr int independent_local(int n){const int x=active()?19:n; return x;}
constexpr int twice(int n){return dependent_local(n)+independent_local(n);}
template<int N> struct Meta {static constexpr int result=active()?N:0;};
template<int N> int choose(){return active()?N:0;}
static_assert(fixed_local()&&required_local(),"");
static_assert(dependent_local(8)==8&&independent_local(8)==19,"");
int main(){
 bool a[]={fixed_local(),required_local(),dependent_local(8)==17,independent_local(8)==19,twice(8)==36};
 constexpr bool b[]={fixed_local(),required_local(),dependent_local(8)==8,independent_local(8)==19,twice(8)==27};
 for(int i=0;i<5;++i)if(!a[i]||!b[i])return i+1;
 if(Meta<4>::result!=4||choose<4>()!=0)return 6;
 return 0;
}
