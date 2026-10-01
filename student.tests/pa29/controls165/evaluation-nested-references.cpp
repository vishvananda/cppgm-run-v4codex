constexpr bool active(){return __builtin_is_constant_evaluated();}
struct Pair {int a,b;};
constexpr int scalar(){const int& r=active()?3:7;return r;}
constexpr int local(){int a=3,b=7;const int& r=active()?a:b;return r;}
constexpr int member(){Pair p{3,5};const int& r=active()?p.b:p.a;return r;}
constexpr int array(){int a[]={2,4,6};const int& r=active()?a[2]:a[0];return r;}
constexpr int write(){int a=3,b=7;int& r=active()?a:b;r=11;return a;}
constexpr int temporary(){int&& r=active()?3:7;r+=8;return r;}
constexpr int bump(int& r){return ++r;}
constexpr int caller(){int a=3,b=7;int& r=active()?a:b;return bump(r)+a;}
static_assert(temporary()==11&&caller()==8,"temporary/caller mutation");
static_assert(scalar()==3,"scalar");
static_assert(local()==3,"local");
static_assert(member()==5,"member");
static_assert(array()==6,"array");
static_assert(write()==11,"write");
int main(){int a[]={scalar(),local(),member(),array(),write(),temporary(),caller()};
if(a[0]!=3||a[1]!=3||a[2]!=5||a[3]!=6||a[4]!=11||a[5]!=11||a[6]!=8)return 1;
if(scalar()!=3||local()!=3||member()!=5||array()!=6||write()!=11)return 2;
return 0;}
