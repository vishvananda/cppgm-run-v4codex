struct Empty { ~Empty() {} };
struct Unused : Empty { ~Unused() {} };
inline void unused() { Unused value; }
int main() { return 0; }
