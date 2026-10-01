template<class T> struct box {};
template<class... U> box(U...,int)->box<int>;
int main(){return 0;}
