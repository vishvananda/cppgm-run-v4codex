struct Base {
  long first,second;
  Base(long n):first(n),second(n+1) {}
  long sum(int n) const { return first+second+n; }
  long difference(int n) const { return second-first+n; }
  long& select() { return second; }
};
struct Derived:Base { Derived(long n):Base(n) {} };
typedef long (Base::*Method)(int) const;
typedef long Base::*Field;
Method forward(Method p) { return p; }
long invoke(Base const& value,Method p,int n) { return (value.*p)(n); }
long read(Base const& value,Field p) { return value.*p; }
int run(bool first) {
  Derived value(7);
  Method p=first ? &Base::sum : &Base::difference;
  Method q=forward(p);
  if(invoke(value,q,3)!=(first?18:4))return 1;
  Base* pointer=&value;
  if((pointer->*q)(5)!=(first?20:6))return 2;
  Field field=&Base::second;
  value.*field=12;
  if(read(value,field)!=12)return 3;
  long& (Base::*select)()=&Base::select;
  (value.*select)()=20;
  return value.second!=20;
}
int main() { return run(false) || run(true); }
