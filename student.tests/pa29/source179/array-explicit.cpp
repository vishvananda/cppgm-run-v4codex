int copies;struct Item{int v;Item(int n=2):v(n){}explicit Item(const Item& a):v(a.v){++copies;}};
int main(){Item arr[2];auto [a,b](arr);return copies!=2||a.v+b.v!=4;}
