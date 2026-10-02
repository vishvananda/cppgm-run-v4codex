#include "vbase.h"
struct Padding { long words[5]; };
struct D : Padding, B { D() { value = 31; } };
int complete();
int main() { D d; B& b = d; return complete() == 17 && b.read() == 31 ? 0 : 1; }
