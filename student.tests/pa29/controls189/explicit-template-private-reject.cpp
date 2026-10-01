class condition { constexpr explicit operator bool() const { return true; } };
template<class T> struct value { explicit(T{}) value() {} };
int main() { value<condition> v; }
