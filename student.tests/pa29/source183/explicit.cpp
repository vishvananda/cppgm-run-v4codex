template<class T> struct box {};
explicit box(int) -> box<int>;
explicit(false) box(double) -> box<double>;
explicit(true) box(char) -> box<char>;
template<class T> explicit(sizeof(T) > 4) box(T*) -> box<T*>;
int main(){return 0;}
