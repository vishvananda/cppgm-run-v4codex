template<class... T> int left(T... x) { return (... - x); }
template<class... T> int right(T... x) { return (x - ...); }
template<class... T> int init_left(T... x) { return (100 - ... - x); }
template<class... T> int init_right(T... x) { return (x - ... - 100); }
template<class... T> bool all(T... x) { return (x && ...); }
template<class... T> bool any(T... x) { return (... || x); }
template<class... T> void nothing(T... x) { (x, ...); }
int main(int argc, char**) {
 nothing();
 return left(argc*20,3,2)!=15 || right(argc*20,3,2)!=19 || init_left(3,2)!=95 || init_right(3,2)!=101 || init_left()!=100 || init_right()!=100 || left(argc)!=argc || !all() || any();
}
