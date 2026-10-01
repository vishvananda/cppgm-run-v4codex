template<class T> struct box { typename T::missing value; };
box(int)->box<int>;
template<class T> box(T)->box<T>;
template<class T> struct forward;
forward(int)->forward<int>;
int main(){return 0;}
