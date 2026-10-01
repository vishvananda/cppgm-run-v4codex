class condition { constexpr explicit operator bool() const { return true; } };
template<class T> struct value { value(T) {} };
explicit(condition{}) value(int) -> value<int>;
int main() { value<int> v(1); }
