template<class T> int choose(T) { return 1; }
template<class T> int choose(T*) { return 2; }
extern template int choose(int*);
template int choose(long*);
template int choose<int>(int*);
int main() { int i=0; long j=0; return choose(&i)+choose(&j)-4; }
