int calls;
template<class T> int operand() { ++calls; return sizeof(T); }
template<class T> int init(int n=operand<T>()) { return n; }
template<class T> struct Box { inline static int values[]={init<T>()}; };
static_assert(sizeof(Box<int>::values)==sizeof(int), "definition demand");
int main() { return Box<int>::values[0]==sizeof(int) && calls==1 ? 0 : 1; }
