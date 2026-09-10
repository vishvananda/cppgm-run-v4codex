struct Value { int member; };
struct Nested { Value objects[2]; };
int main() {
    volatile Value&& object = {7};
    Value&& ordinary = {8};
    volatile Nested&& nested = {{{9}, {10}}};
    return object.member != 7 || ordinary.member != 8 || nested.objects[0].member != 9 || nested.objects[1].member != 10;
}
