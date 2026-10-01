template<class T> struct box {};
box(int = 4) -> box<int>;
template<class T = double> box(T = T{}) -> box<T>;
template<class... T> box(char, T... args) -> box<int>;
int main(){return 0;}
