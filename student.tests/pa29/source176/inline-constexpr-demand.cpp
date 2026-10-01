template<class T> struct Constants {
 inline static constexpr int a = sizeof(T);
 inline static constexpr int b = a+2;
};
static_assert(Constants<int>::b == sizeof(int)+2,"constant demand only");
int main() { return 0; }
