template<class T> struct Box { inline static int values[]={T::missing}; };
int main() { if constexpr(false) return sizeof(Box<int>::values); }
