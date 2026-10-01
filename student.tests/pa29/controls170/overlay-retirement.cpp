struct P {int* a;int* b;};
constexpr int read(const P& p){return *p.a+*p.b;}
constexpr int work(){int a=1,b=2,c=3;P p={&a,&b};int sum=0;
for(int i=0;i<20;++i){p={&a,&b};p.b=&c;sum+=read(p);p={&b,&a};p.a=&c;sum+=read(p);c+=1;}
return sum;}
static_assert(work()==540,"retired overlays do not leave stale dependencies");
int main(){return work()!=540;}
