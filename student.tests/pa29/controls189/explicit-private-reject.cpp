class condition { constexpr explicit operator bool() const { return true; } };
struct value { explicit(condition{}) value() {} };
int main() { value v; }
