template<class T> struct box {};
extern "C" {box(int)->box<int>;}
int main(){return 0;}
