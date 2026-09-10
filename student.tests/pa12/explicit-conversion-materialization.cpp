int live, moves, calls;
struct Item {
  int value;
  Item(int n):value(n){++live;}
  Item(Item&& x):value(x.value){if(live<1)__builtin_abort();++live;++moves;x.value=-1;}
  ~Item(){--live;}
};
struct Source {
  int value;
  explicit operator Item()const {++calls;return Item(value);}
};
Item convert(Source x){return Item(x);}
struct Box{Item value;Box(Source x):value(x){}};
struct Empty{};
int empty_calls;
struct EmptySource {explicit operator Empty()const{++empty_calls;return Empty();}};
Empty empty(EmptySource x){return Empty(x);}
int main(){
  Source s={17};
  {
    Item a(s);if(a.value!=17||live!=1||moves!=1||calls!=1)return 1;
    Item b=Item(s);if(b.value!=17||live!=2||moves!=2||calls!=2)return 2;
    Item c=(Item)s;if(c.value!=17||live!=3||moves!=3||calls!=3)return 3;
    Item d=convert(s);if(d.value!=17||live!=4||moves!=4||calls!=4)return 4;
    Box e(s);if(e.value.value!=17||live!=5||moves!=5||calls!=5)return 5;
    EmptySource input;Empty result=empty(input);if(empty_calls!=1)return 6;
  }
  return live!=0;
}
