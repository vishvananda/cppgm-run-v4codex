int live;
int value = 23;
struct Value {
    int& reference;
    int* pointer;
    Value() : reference(value), pointer(&value) { ++live; }
    ~Value() { --live; }
    operator int&() { return value; }
};
const int& global = Value().reference;
int& call(Value&& object) { return object.reference; }
int main() {
    if (live || global != 23) return 1;
    const int& member = Value().reference;
    if (live || member != 23) return 2;
    const int& element = Value().pointer[0];
    if (live || element != 23) return 3;
    const int& converted = Value();
    if (live || converted != 23) return 4;
    const int& result = call(Value());
    if (live || result != 23) return 5;
    const int& reversed = 0[Value().pointer];
    return live || reversed != 23;
}
