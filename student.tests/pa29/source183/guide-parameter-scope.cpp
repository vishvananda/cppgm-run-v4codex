template<int N> struct sized {};
int count;
sized(int count) noexcept(noexcept(count+1)) -> sized<sizeof(count)>;
template<class T> struct box {};
template<class T> box(T value) -> box<decltype(value)>;
int main(){return count;}
