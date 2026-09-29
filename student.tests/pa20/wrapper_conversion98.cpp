// Reduced rejection proof: closure -> function pointer -> Wrapper needs two
// user-defined conversions in one implicit conversion sequence.
struct Wrapper { Wrapper(int (*)()); };
void use(const Wrapper&);
int main() { use([] { return 1; }); }
