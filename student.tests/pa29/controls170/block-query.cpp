typedef int (^B)(int);
struct S{B b;};
static_assert(noexcept((S){.b=nullptr}),"compound block member");
template<class T> T make(){return (T){.b=nullptr};}
int main(){S s=make<S>();return s.b!=nullptr;}
