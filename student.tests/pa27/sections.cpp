extern int *first __attribute__((section("records")));
__attribute__((section("records"))) int *first = 0;
int outside = 41;
__attribute__((section("records"))) int *second = &outside;
alignas(32) __attribute__((section("records"))) long wide[4] = {2,3,5,7};
template<class T> struct Box { static T value; };
template<class T> __attribute__((section("records"))) T Box<T>::value = 9;
int main()
{
    return !first && *second == 41 && wide[3] == 7 &&
        reinterpret_cast<unsigned long>(wide) % 32 == 0 && Box<int>::value == 9 ? 0 : 1;
}
