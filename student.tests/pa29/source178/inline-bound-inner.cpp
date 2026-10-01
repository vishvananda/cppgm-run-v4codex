template<class T> int dormant() { return T::missing; }
template<class T> struct Box { inline static int values[]={sizeof(dormant<T>())}; };
static_assert(sizeof(Box<int>::values)==sizeof(int), "bound demand");
int main() { return Box<int>::values[0]==sizeof(int) ? 0 : 1; }
