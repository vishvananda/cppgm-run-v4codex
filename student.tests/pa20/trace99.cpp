struct Member { int n; Member() : n(7) {} };
template<int N> struct Box { int prefix; Member value; };
template<int N> Box<N> make(int n) { return {n}; }
Box<3> other(int n) { return {n}; }
struct Move { int n; Move(int n) : n(n) {} Move(Move&&) = default; };
Move moved(int n) { return Move(n); }
Move forward(int n) { return moved(n); }
int main() {
    Box<3> a = make<3>(1), b = make<3>(2), c = other(3);
    Box<4> d = make<4>(4), e = make<4>(5);
    Move m = forward(6);
    return a.prefix != 1 || b.prefix != 2 || c.prefix != 3 ||
        d.prefix != 4 || e.prefix != 5 || a.value.n != 7 ||
        b.value.n != 7 || c.value.n != 7 || d.value.n != 7 ||
        e.value.n != 7 || m.n != 6;
}
