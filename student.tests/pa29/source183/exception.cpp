template<class T> struct box {};
box(int) noexcept -> box<int>;
box(double) noexcept(false) -> box<double>;
template<class T> box(T x) noexcept(noexcept(x+x)) -> box<T>;
int main(){return 0;}
