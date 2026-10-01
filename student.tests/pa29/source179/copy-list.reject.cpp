#include <initializer_list>
struct Pair{int a,b;};int main(){Pair p{1,2};auto [x,y]={p};}
