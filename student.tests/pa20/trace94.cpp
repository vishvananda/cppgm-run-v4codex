template<class T> struct Counter {
    T value;
    operator T&() { return value; }
};
template<class T> auto&& alias(T& value) { return value; }
template<class... T> auto sum(T... values) {
    int result = 0;
    using Array = int[];
    (void)Array{0, (result += values, 0)...};
    return result;
}
struct Pair { int first; int second; };
auto make_pair(int first) { return Pair(first); }
int main() {
    Counter<long> count = {7};
    if (auto pointer = &count.value) {
        auto& reference = alias(*pointer);
        long before = count++;
        Pair pair = make_pair(sum(2, 3));
        return &reference != pointer || reference != 8 || before != 7 ||
               pair.first != 5 || pair.second != 0;
    }
    return 1;
}
