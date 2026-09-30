template<bool, class T> struct Enable {};
template<class T> struct Enable<true,T> { typedef T type; };
template<class T> class Trait {
    static const bool value=true;
    template<class> friend struct First;
};
template<class T> struct First {
    template<class U> friend typename Enable<Trait<U>::value,U>::type
    pick(First, U value) { return value+1; }
};
int main() { First<long> f; return pick(f,16)-17; }
