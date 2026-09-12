// Static vptr state and deleting entries consume the same prepared lifetimes.
int destroyed = 0;
template<class T> struct Global {
    T value;
    virtual int read() { return value; }
    virtual ~Global() {}
};
Global<int> global;
struct Base { virtual ~Base() { ++destroyed; } };
template<class T> struct Trivial { typename T::tag* pointer; };
struct Derived : Base { typedef int tag; Trivial<Derived> field; };
struct Tag;
int chosen(const Tag&);
int before_definition(const Tag& tag) { return chosen(tag); }
struct Tag {
    friend int chosen(const Tag&) { return 9; }
    friend int unused(const Tag&, int) { return 11; }
};
// Later variable-template forms remain parsed, without demanding storage.
template<class T, class U, class = void> const bool equal_types = false;
template<class T> const bool equal_types<T, T> = true;
int main() {
    Global<int>* pointer = &global;
    if (pointer->read() != 0) return 1;
    Base* owned = new Derived;
    delete owned;
    if (destroyed != 1) return 2;
    Tag tag;
    return before_definition(tag) != 9;
}
