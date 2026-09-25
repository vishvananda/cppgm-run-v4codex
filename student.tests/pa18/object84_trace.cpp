int effects;
template<class T>struct Base {
  T value;
  Base():value(7) {++effects;}
  template<class U>Base(U):Base() {++effects;}
};
struct Derived:Base<int> {using Base<int>::Base;};
struct Empty {~Empty(){++effects;}};
template<class T>int take(T){return effects;}
struct Factory {template<class T>int size(T){return sizeof(T);}};
long scale(long x){return x*3;}
int main(){
  Derived d(1); Base<int> b(1);
  if(d.value!=7||b.value!=7||effects!=4)return 1;
  {Empty e{};if(take(e)!=4||effects!=5)return 2;}
  if(effects!=6)return 3;
  struct Tag{};Factory f;
  return f.size(Tag{})!=1||scale(-7)!=-21;
}
