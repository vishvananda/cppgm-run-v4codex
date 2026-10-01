template<class T> struct box {};
box(int (*p)(double))->box<int>;
box(int (&a)[3])->box<int*>;
box(void)->box<void>;
box(char,...)->box<char>;
int main(){return 0;}
