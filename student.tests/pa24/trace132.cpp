template<int N> struct Counter {
    long advance(long n) { return n + N; }
    long dormant() { return N.missing; }
};
thread_local long count = 7;
int main(int argc, char**) {
    Counter<3> counter;
    long* address = &count;
    count = counter.advance(count + argc);
    return *address != 10 + argc;
}
