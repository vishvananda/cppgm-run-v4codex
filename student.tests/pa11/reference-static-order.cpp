int value = 7;
extern int& value_ref;
int observe() { return value_ref; }
int observed = observe();
int& value_ref = value;
int main() {
    value_ref = 9;
    return observed == 7 && value == 9 ? 0 : 1;
}
