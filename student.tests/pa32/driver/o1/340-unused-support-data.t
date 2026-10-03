struct Unused {
    virtual ~Unused() {}
    virtual int value() { return 7; }
};
inline void unused_object() { Unused value; }
static int hidden() { return 33; }
static int (*unused_pointer)() = hidden;
int main() { return 0; }
