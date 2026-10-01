template<class T> char f(T(*)[0]);
template<class T> long f(...);
static_assert(sizeof(f<int>((int(*)[0])0))==sizeof(long),"forming a dependent zero array is a substitution failure");
int main(){return 0;}
