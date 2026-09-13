// Fixed conversion decisions still retain O0 constant-width policy after a
// value-dependent layout query is substituted with a concrete operand type.
template<class T> unsigned long left() { return 1+sizeof(T); }
template<class T> unsigned long right() { return sizeof(T)+1; }
template<class T> unsigned long aligned() { return 1+__alignof(T); }
template<class T> struct Box { T value; };
int main() {
    return left<int>()!=5 || right<long>()!=9 || aligned<int>()!=5 ||
        left<Box<int>>()!=5 || right<Box<long>>()!=9;
}
