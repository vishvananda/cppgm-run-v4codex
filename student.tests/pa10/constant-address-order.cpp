// [basic.start.init]/2: middle is constant-initialized before observed's call.
int values[3];
extern int *middle;
int *read_middle() { return middle; }
int *observed = read_middle();
int *middle = &values[1];
int main() { return observed == &values[1] ? 0 : 1; }
