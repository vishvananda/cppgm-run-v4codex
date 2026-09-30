int destroyed;
struct Empty { ~Empty() {} };
struct Wrapper { Empty e; };
struct Effect { ~Effect() { ++destroyed; } };
void fail() { throw 7; }
struct Object {
    Effect first;
    Wrapper second;
    Object() { fail(); }
};
int main() {
    try { Object o; } catch (int n) { return n==7 && destroyed==1 ? 0:1; }
    return 2;
}
