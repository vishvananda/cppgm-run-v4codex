struct P {int* a;int* b;};
constexpr int read(int* const& a,int* const& b){return *a+*b;}
constexpr int work(){int x=1,y=2,z=3;P p={&x,&y};p.b=&z;int first=read(p.a,p.b);z=9;return first+read(p.a,p.b);}
static_assert(work()==14,"changed pointee of second overlaid sibling");
int main(){return work()!=14;}
