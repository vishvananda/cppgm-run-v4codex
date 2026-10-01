class condition { constexpr explicit operator bool() const { return true; } };
struct value { template<class T> explicit(T{}) value(T) {} };
int main() { value v(condition{}); }
