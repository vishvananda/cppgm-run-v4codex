struct aggregate {
  int value;
  constexpr explicit operator bool()const{return value==3;}
};
struct constructed {
  int value;
  constexpr constructed(int x,int y=2):value(x+y){}
  constexpr explicit operator bool()const{return value==3;}
};
struct nested {
  aggregate a[2];
  constexpr explicit operator bool()const{return bool(a[0])&&bool(a[1]);}
};
struct bits {
  unsigned int value:2;
  constexpr explicit operator bool()const{return value==1;}
};
struct text {
  char value[3];
  constexpr explicit operator bool()const{return value[0]=='o'&&value[1]=='k'&&!value[2];}
};
constexpr int three=3;
struct reference {
  const int& value;
  constexpr explicit operator bool()const{return value==3;}
};
template<class T> int check(){
  static_assert(aggregate{3},"fixed aggregate");
  static_assert(constructed{1},"fixed constructor default");
  static_assert(nested{{{3},{3}}},"fixed nested arrays");
  static_assert(bits{5},"bitfield conversion");
  static_assert(text{"ok"},"literal array");
  static_assert(reference{three},"reference member");
  static_assert(T{3},"dependent aggregate");
  return 0;
}
int main(){return check<aggregate>();}
