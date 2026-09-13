// N3485 [temp.local], [dcl.type.simple], [dcl.enum], [basic.life], [class.dtor].
// Local declarations have distinct identities per specialization and scope;
// aliases/layout queries reuse that identity without sharing runtime objects.
int destructions;
template<class T> struct Counter { static int value; };
template<class T> int Counter<T>::value = 0;
template<class T> int* identity_counter() { return &Counter<T>::value; }

template<class T> int local_facts() {
    struct Local {
        typedef T value_type;
        T value;
        Local(T x) : value(x) {}
        Local(const Local& x) : value(x.value+1) {}
        ~Local() { ++destructions; }
    };
    typedef Local Alias;
    Alias first(T(5));
    Alias second(first);
    const Alias* pointer = &second;
    typedef decltype(first) Object;
    typedef decltype((first)) Reference;
    Reference reference = first;
    enum Extent { Count = sizeof(Local), Twice = 2*Count };
    Extent count = Count;
    int storage[Twice];
    storage[0] = pointer->value;
    struct Nest { struct Part { T value; }; };
    typename Nest::Part part;
    part.value = reference.value;
    int* outer = identity_counter<Local>();
    ++*outer;
    {
        struct Local { long value; };
        int* inner = identity_counter<Local>();
        if (inner == outer || *inner) return 1;
        ++*inner;
    }
    if (sizeof(Object) != sizeof(T)) return 10;
    if (alignof(Object) != alignof(T)) return 11;
    if (sizeof(storage) != 2*sizeof(T)*sizeof(int)) return 12;
    if (count != sizeof(T)) return 13;
    if (sizeof(typename Nest::Part) != sizeof(T)) return 14;
    if (part.value != 5) return 15;
    if (storage[0] != 6) return 16;
    if (*outer != 1) return 17;
    if (pointer != &second) return 18;
    return 0;
}

int main() {
    int first = local_facts<int>(); if (first) return first;
    if (destructions != 2) return 60;
    int second = local_facts<long>(); if (second) return second;
    if (destructions != 4) return 61;
    return 0;
}
