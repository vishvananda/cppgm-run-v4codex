// [basic.start.init]/2: binding alias precedes every dynamic initializer.
extern const int &alias;
const int *read_alias() { return &alias; }
const int *observed = read_alias();
const int value = 42;
const int &alias = value;
int main() { return observed == &value ? 0 : 1; }
