int main() {
    const int rows[2][2]={{1,2},{3,4}};
    int sum=0;
    for(auto [x,y]:rows) {
        static_assert(__is_same(decltype(x),const int),"const range element");
        sum+=x+y;
    }
    return sum!=10;
}
