template<class T> int copy(T& data) {
    auto [x,y] = data;
    static_assert(__is_same(decltype(x),const int),"const array element");
    return x+y;
}
int main() { const int a[2]={3,4}; return copy(a)!=7; }
