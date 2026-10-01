int calls;
template<class T> int init() { ++calls; return 17; }
template<class T> struct Box { inline static int values[]={init<T>()}; };
static_assert(sizeof(Box<int>::values)==sizeof(int), "bound demand");
int main() { return Box<int>::values[0]==17 && calls==1 ? 0 : 1; }
