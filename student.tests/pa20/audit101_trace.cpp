int live;
struct Range {
    int values[2];
    Range() : values{3,4} { ++live; }
    ~Range() { --live; }
    auto begin() { return values; }
    auto end() { return values+2; }
};
template<class T> int sum(T (&values)[2]) {
    auto outer = [&]() {
        auto inner = [&]() {
            int result = 0;
            for (const auto& value : values) result += value;
            return result;
        };
        return inner();
    };
    return outer();
}
int main() {
    int a[2] = {3,4}; long b[2] = {5,6};
    if (sum(a)!=7 || sum(b)!=11 || sum(a)!=7) return 1;
    {
        Range range;
        auto update = [&]() {
            int result = 0;
            for (auto& value : range) { result += value; ++value; }
            return result;
        };
        if (update()!=7 || range.values[0]!=4 || range.values[1]!=5 || live!=1) return 2;
    }
    return live;
}
