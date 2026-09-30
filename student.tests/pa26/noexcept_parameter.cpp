template<class T> struct Wrap {
    typedef T value_type;
    explicit Wrap(value_type x) noexcept(noexcept(T(x))) : value(x) {}
    T value;
};
static_assert(noexcept(Wrap<int>(1)),"parameter type in exception specification");
int main(){return Wrap<int>(7).value==7 ? 0:1;}
