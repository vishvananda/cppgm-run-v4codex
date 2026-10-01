template<int N> constexpr unsigned length(const char(&)[N]) { return N-1; }
constexpr bool active(){return __builtin_is_constant_evaluated();}
int destroyed;
struct Box {int value; ~Box(){++destroyed;}};
Box make(int x){return Box{x};}
template<int N> int work() {
    constexpr unsigned name_size = length(__PRETTY_FUNCTION__);
    static_assert(name_size > length(__func__), "signature contains function name");
    const int& mode = active() ? N : 0;
    int flag = 0;
    asm("movl %1,%0" : "=r"(flag) : "r"(mode));
    int selected = flag ? make(N).value : make(0).value;
    return selected + name_size - length(__PRETTY_FUNCTION__);
}
int main(){return work<3>() == 3 && work<5>() == 5 && destroyed > 0 ? 0 : 1;}
