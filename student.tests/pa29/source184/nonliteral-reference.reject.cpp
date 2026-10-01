struct Tracked {int value; constexpr Tracked(int n):value(n){} ~Tracked(){}};
static_assert(const_cast<Tracked&&>(Tracked(7)).value==7,"nonliteral temporary");
int main(){}
