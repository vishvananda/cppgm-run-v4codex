// Pending source-EH boundary: ordinary reference/typeid success still links
// throwing failure helpers. These must share typed exceptions with catch/rethrow.
namespace std { class type_info; }
struct A { virtual ~A(){} };
struct B:A {};
int main(){B b; A* a=&b; return &dynamic_cast<B&>(*a)!=&b || &typeid(*a)!=&typeid(B);}
