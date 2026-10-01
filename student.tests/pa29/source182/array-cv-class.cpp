int copies,dead;
struct Item { int n; Item(int x):n(x){} Item(const Item& x):n(x.n){++copies;} ~Item(){++dead;} };
int main() {
    { const Item source[2]={3,4};
      { auto [x,y]=source;
        static_assert(__is_same(decltype(x),const Item),"const class element");
        if(x.n+y.n!=7 || copies!=2)return 1;
      }
      if(dead!=2)return 2;
    }
    return dead!=4;
}
