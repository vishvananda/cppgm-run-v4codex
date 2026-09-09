int live = 0, moved = 0;
struct Move {
    int value;
    Move(int n) : value(n) { ++live; }
    Move(Move&& x) : value(x.value) { x.value = 0; ++live; ++moved; }
    Move(Move const&) = delete;
    ~Move() { --live; }
};
struct Aggregate { int prefix; Move member; };
Aggregate make() { return Aggregate{3,Move(7)}; }
int main() {
    {
        Aggregate value = make();
        if (live != 1 || value.prefix != 3 || value.member.value != 7) return 1;
    }
    return live != 0;
}
