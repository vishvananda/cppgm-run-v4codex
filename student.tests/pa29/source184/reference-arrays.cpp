constexpr int values[3]={4,5,6};
constexpr int (&view)[3] = (int(&)[3])values;
constexpr int (&named)[3] = const_cast<int(&)[3]>(values);
static_assert(&view[1]==&values[1] && named[2]==6,"array reference");
struct Box { int value; constexpr Box(int v):value(v){} };
constexpr int temporary = const_cast<Box&&>(Box(19)).value;
static_assert(temporary==19,"class prvalue reference");
int f(int n) {return n+1;}
int main(){
 int x=9; const int& r=x; (int&)r=11;
 int (* const fp)(int)=f;
 int (*&rp)(int)=const_cast<int(*&)(int)>(fp);
 return x==11 && rp(4)==5?0:1;
}
