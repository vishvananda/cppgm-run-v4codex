// N3485 1.3.18, 7.1.6.2/4, 14.5.6.1/5-6, 14.6.3/1.
// Both decltype operands are nondependent. Return types distinguish templates.
long select(double);
template<class T> auto result(T) -> decltype(select(0));
int select(int);
template<class T> auto result(T) -> decltype(select(0));
static_assert(sizeof(result(0)) == sizeof(long), "ambiguous call must reject");
int main() {}
