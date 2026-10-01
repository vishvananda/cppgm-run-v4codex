constexpr int read(int** p){return **(p+1);}
constexpr int work(){int a=1,b=2,c=3;int* p[2]={&a,&b};p[1]=&c;int x=read(p);c=7;return x+read(p);}
static_assert(work()==10,"overlaid sibling reached by pointer arithmetic");
int main(){return work()!=10;}
