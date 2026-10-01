constexpr const char* file(const char* p=__builtin_FILE()) { return p; }
#line 20 "alpha.cpp"
const char* a=file();
#line 30 "beta.cpp"
const char* b=file();
struct Guard { ~Guard() {} } guard;
int main() { return a[0]=='a' && b[0]=='b' ? 0 : 1; }
