struct Item{int v;Item(int n=2):v(n){}explicit Item(const Item& a):v(a.v){}};int main(){Item arr[2];auto [a,b]=arr;}
