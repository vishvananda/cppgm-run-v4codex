template<class T> struct Storage { enum {capacity=15/sizeof(T)};union{T data[capacity+1];unsigned long other;};};
static_assert(sizeof(Storage<char>)==16,"dependent enum bound");
int main(){Storage<char> s;s.data[15]=3;return s.data[15]!=3;}
