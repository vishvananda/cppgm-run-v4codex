template<class T> struct box {};
template<class T> box(T)->box<T>;
template<class T> box(T)->box<int>;
box(int)->box<char>;
box()->box<void>;
int main(){return 0;}
