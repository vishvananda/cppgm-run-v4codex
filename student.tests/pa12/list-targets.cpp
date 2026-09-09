struct Point { int x, y; };
int pick(int) { return 1; }
int pick(Point) { return 2; }
int bind(Point const&) { return 1; }
int bind(Point&& p) { return p.x + p.y; }
struct Default { int n; Default() : n(7) {} };
int use(Default d = {}) { return d.n; }
int reference(int const (&a)[4]) { return a[0] + a[1] + a[2] + a[3]; }
int main() {
    int x = 5;
    int& alias{x}; alias = 9;
    int const& zero{};
    Point const& p{3,4};
    return pick({3}) == 1 && pick({3,4}) == 2 && bind({3,4}) == 7 &&
        use() == 7 && x == 9 && zero == 0 && p.y == 4 && reference({1,2}) == 3 ? 0 : 1;
}
