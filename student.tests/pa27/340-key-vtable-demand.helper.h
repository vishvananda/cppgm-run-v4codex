struct Exported { virtual int value(); };
struct Unused {
    virtual ~Unused() {}
    virtual int value() { return 3; }
};
inline void unused_object() { Unused value; }
