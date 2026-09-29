namespace owner { struct C{ int x; }; int find(int C::*) {return 4;} }
namespace result { struct R{}; int pick(R owner::C::*) {return 5;} }
template<class T> int use(T p){ return find(p); }
template<class T> int get(T p){ return pick(p); }
int main(){ result::R owner::C::*p=nullptr; return use(&owner::C::x)!=4 || get(p)!=5; }
