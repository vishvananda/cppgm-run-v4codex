int sequence = 0;
int mark(int value) { sequence = sequence * 10 + value; return value; }
struct Base {
    int first;
    Base(int v = mark(1)) : first(v) {}
};
struct Member {
    int value;
    Member(int v) : value(v) {}
};
struct Object : Base {
    int second = mark(2);
    Member member;
    int &alias;
    Object(int &r, int v = 7) : alias(r), member(v) {}
    int sum() const { return first + second + member.value; }
};
int shared = 4;
Object global(shared);
int main() {
    if (sequence != 12 || global.sum() != 10) return 1;
    global.alias = 9;
    if (shared != 9) return 2;
    sequence = 0;
    Object local(shared, 11);
    if (sequence != 12 || local.sum() != 14) return 3;
    local.alias = 15;
    return shared == 15 ? 0 : 4;
}
