// PA34 reducer: an elaborated type use must not hide a same-named function.
// C++11 [dcl.type.elab] p2, [basic.lookup.elab]: lookup finds the visible class;
// only a standalone forward declaration introduces a new class in this scope.
struct action { int value; };
int action(int number, struct action* input, struct action* output) {
    if (output) output->value = input->value + number;
    return input->value + number;
}
int main() {
    int sig = 3;
    struct action reset = {7};
    action(sig, &reset, 0);
    if (action(sig, &reset, 0) != 10) return 1;
    return 0;
}
