template<bool, class T> struct Enable {};
template<class T> struct Enable<true,T> { typedef T type; };
template<class T> struct Trait { static const bool value=true; };
template<class T> struct First {
    template<class U> friend typename Enable<Trait<U>::value,U>::type pick(U);
};
template<class T> struct Second {
    template<class U> friend typename Enable<Trait<U>::value,U>::type pick(U);
};
template<class U> typename Enable<Trait<U>::value,U>::type pick(U value) { return value+1; }
int main() { Second<int> second; First<long> first; return pick(16)-17; }
