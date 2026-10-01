template<class T> struct Context {
 inline static const char* function = __builtin_FUNCTION();
 inline static int source_line = __builtin_LINE();
private:
 static int hidden() { return 7; }
public:
 inline static int value = hidden();
};
int main() { return Context<int>::function[0] == 0 && Context<int>::source_line == 3 && Context<int>::value == 7 ? 0 : 1; }
