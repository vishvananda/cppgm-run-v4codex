namespace left { int main() { return 13; } }
namespace right { int main() { return 17; } }
struct member { static int main() { return 19; } };
// Exercise collision allocation as well as the reserved entry spelling.
int main__ordinary() { return 23; }
int main() {
    return left::main()+right::main()+member::main()+main__ordinary() == 72 ? 0 : 1;
}
