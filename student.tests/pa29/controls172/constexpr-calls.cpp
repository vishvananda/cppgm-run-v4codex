template<class... T> constexpr int sum(T... x){return (0+...+x);}
static_assert(sum(1,2,3)==6,"one");
static_assert(sum(10,20)==30,"two");
static_assert(sum()==0,"empty");
int main(){return sum(2,3)-5;}
