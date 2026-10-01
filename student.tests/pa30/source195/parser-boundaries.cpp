template<class T> struct truth { constexpr operator bool() const { return true; } };
template<class T> struct wrap : T { typedef int type; };
template<bool B> struct accept { static const bool value = B; };
static_assert(accept<wrap<truth<int>>{}>::value, "both closing pieces belong to type");
static_assert(accept<(wrap<truth<int>>())>::value, "parenthesized nested construction");
static_assert(accept<truth<int>{}>::value, "single close construction");
template<class T> struct outer { static const int value = 11; };
outer<truth<int>> split{};
static_assert(outer<truth<int>>::value == 11, "outer owns second closing piece and qualifier");
template<class T> struct uses { typedef typename T::type type; };
uses<wrap<truth<int>>>::type use;
struct visible { typedef int type; };
struct friend_user { friend class visible; visible::type value; };
struct enclosing { struct visible { typedef char type; }; };
struct qualified_friend { friend struct enclosing::visible; enclosing::visible::type value; };
template<class T> struct templated { typedef T type; };
struct template_friend {
    template<class U> friend struct templated;
    templated<int>::type value;
};
template<class T> struct assigned {
    int value;
    assigned() : value(0) {}
    template<class U> assigned& operator=(const U& v) { value = v; return *this; }
    typedef T type;
    template<class U> static constexpr int result = sizeof(U);
    int after() { return value; }
};
int main() {
    assigned<int> a; a = 13;
    friend_user f; f.value = a.after();
    return f.value != 13 || sizeof(qualified_friend) != 1 || sizeof(template_friend) != sizeof(int);
}
