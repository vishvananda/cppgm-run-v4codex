class condition { constexpr explicit operator bool() const { return true; } };
void f() noexcept(condition{}) {}
int main() { f(); }
