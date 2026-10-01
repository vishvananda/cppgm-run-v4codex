template<class T>int copy(T (&a)[2]){auto [x,y]=a;x+=y;return x;}
template<class T,int N>int rows(T (&a)[N][2]){int sum=0;for(auto [x,y]:a)sum+=x+y;return sum;}
int main(){int a[2]={3,4};long b[2]={5,6};int c[2][2]={{1,2},{3,4}};return copy(a)!=7||copy(b)!=11||a[0]!=3||rows(c)!=10;}
