int trace;
struct Guard {
    int id;
    Guard(int n) : id(n) {}
    ~Guard() { trace = trace * 10 + id; }
};
int return_value(bool first) {
    Guard a(1);
    Guard b(2);
    if (first) return trace + 7;
    return trace + 9;
}
void exits() {
    for (Guard outer(3); trace == 0;) {
        Guard inner(4);
        break;
    }
    int count = 0;
again:
    Guard repeated(5);
    if (count++ == 0) goto again;
}
int main() {
    trace = 0;
    if (return_value(true) != 7 || trace != 21) return 1;
    trace = 0;
    if (return_value(false) != 9 || trace != 21) return 2;
    trace = 0;
    exits();
    if (trace != 4355) return 3;
    trace = 0;
    for (Guard once(6); trace == 0; trace = 7) {}
    return trace != 76;
}
