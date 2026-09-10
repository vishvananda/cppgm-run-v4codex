int live;
struct Value {
    int member;
    int array[2];
    Value(int n) : member(n) { ++live; }
    ~Value() { --live; }
};
int main() {
    {
        const int& member = Value(7).member;
        if (live != 1 || member != 7) return 1;
    }
    if (live) return 2;
    {
        const int& member = (0, (Value(9).member));
        if (live != 1 || member != 9) return 3;
    }
    if (live) return 4;
    {
        const int& member = Value(1).array[0];
        if (live != 1) return 5;
    }
    if (live) return 6;
    {
        const long& converted = Value(11).member;
        if (live || converted != 11) return 7;
    }
    if (live) return 8;
    {
        const Value& object = static_cast<Value&&>(Value(13));
        if (live != 1 || object.member != 13) return 9;
    }
    if (live) return 10;
    {
        const int& member = 0[Value(1).array];
        if (live != 1) return 11;
    }
    if (live) return 12;
    {
        int Value::*pointer = &Value::member;
        const int& member = Value(17).*pointer;
        if (live != 1 || member != 17) return 13;
    }
    if (live) return 14;
    return 0;
}
