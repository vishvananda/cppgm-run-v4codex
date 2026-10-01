template<class T> struct box {};
extern int external();
box(int = external())->box<int>;
int main(){return 0;}
