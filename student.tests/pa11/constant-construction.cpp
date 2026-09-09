int effects;
struct Direct { const char* name; int value; Direct(const char* n, int v) : name(n), value(v) {} };
Direct direct[] = {{"a",3},{"b",4}};
struct Effect { int value; Effect(int n) : value(n) { ++effects; } };
Effect effect[] = {{5},{6}};
struct Narrow { int value; Narrow(unsigned char n) : value(n) {} };
Narrow narrow[] = {{static_cast<unsigned char>(300)}};
int main() {
    if (direct[1].name[0] != 'b' || direct[0].value != 3 || direct[1].value != 4) return 1;
    if (effects != 2 || effect[1].value != 6) return 2;
    return narrow[0].value == 44 ? 0 : 3;
}
