template<class T> int init() { return T::missing; }
template<class T> struct Box { inline static int values[]={init<T>()}; };
static_assert(sizeof(Box<int>::values)==sizeof(int), "definition demand");
