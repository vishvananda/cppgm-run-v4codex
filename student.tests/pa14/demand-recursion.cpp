// N3485 [temp.inst], [temp.point]: declarations precede body demands;
// unused dependent definitions/defaults remain independent facts.
template<class T> int count(T value) {
    if (!value) return 0;
    return 1+count(T(value-1));
}
template<class T> int late(int value);
int early() { return late<long>(3); }
template<class T> struct Chain {
    Chain* next;
    T value;
    int sum() { return int(value)+(next ? next->sum() : 0); }
    int unused(int value=T::missing) { return T::absent+value; }
};
template<class U> int late(int value) {
    struct Local {
        U value;
        int get() { return int(value); }
    };
    Local object; object.value=U(value);
    Chain<U> second; second.value=U(2); second.next=nullptr;
    Chain<U> first; first.value=U(object.get()); first.next=&second;
    return first.sum()+count(U(value));
}
template<class T> struct Unused { typename T::missing field; };
Unused<int>* never_completed=nullptr;
template<class T> struct Outer { struct Inner; };
template<class U> struct Outer<U>::Inner { U value; int get() { return int(value); } };
int main() {
    if (early()!=8 || late<int>(4)!=10 || count(7)!=7 || count(5L)!=5) return 1;
    Chain<int> object; object.value=11; object.next=nullptr;
    Outer<int>::Inner first; first.value=12;
    Outer<long>::Inner second; second.value=13;
    return object.sum()!=11 || never_completed!=nullptr || first.get()!=12 || second.get()!=13;
}
