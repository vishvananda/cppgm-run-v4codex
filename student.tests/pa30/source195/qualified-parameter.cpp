struct base {
    enum mode_type { read_mode, write_mode };
    typedef mode_type openmode;
};
template<class T> struct stream {
    friend class base;
    typedef stream<T> self;
    self* open(const char*, base::openmode);
};
static_assert(sizeof(stream<char>) == 1, "friend preserves qualified parameter lookup");
int main() { return 0; }
