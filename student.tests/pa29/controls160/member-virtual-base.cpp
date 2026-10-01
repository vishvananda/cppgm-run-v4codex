struct Base{int x; int add(int y) const {return x+y;}};
struct L:virtual Base{}; struct R:virtual Base{}; struct D:L,R{};
struct Ptr{D* p; D& operator*() const {return *p;}};
int main(){D d;d.x=5;Ptr p={&d};
 if(__builtin_invoke(&Base::add,d,3)!=8)return 1;
 __builtin_invoke(&Base::x,&d)=6;
 return __builtin_invoke(&Base::add,p,3)==9?0:2;}
