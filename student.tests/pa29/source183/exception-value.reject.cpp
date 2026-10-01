template<class T> struct box {};
extern bool state;box(int) noexcept(state)->box<int>;
