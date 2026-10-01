int copies,dead;
struct Z { int empty[0]; Z(){} Z(const Z&){++copies;} ~Z(){++dead;} };
int main() {
    { const Z source[2];
      { auto [x,y]=source;
        static_assert(__is_same(decltype(x),const Z),"cv zero-sized element");
        if(copies!=2 || sizeof(x)!=0)return 1;
      }
      if(dead!=2)return 2;
    }
    return dead!=4;
}
