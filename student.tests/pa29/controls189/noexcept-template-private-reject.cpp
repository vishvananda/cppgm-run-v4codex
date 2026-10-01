class condition { constexpr explicit operator bool() const { return true; } };
template<class T> void f() noexcept(T{}) {}
int main() { f<condition>(); }
