namespace copy_only {
struct S { int value; S(int x):value(x){} S(const S& s):value(s.value){} };
int take(S s) { return s.value; }
S make(int x) { return S(x); }
}
namespace movable {
struct S { int value; S(int x):value(x){} S(const S& s):value(s.value){} S(S&& s):value(s.value){} };
int take(S s) { return s.value; }
S make(int x) { return S(x); }
}
namespace destruction {
struct S { int value; S(int x):value(x){} ~S(){} };
int take(S s) { return s.value; }
S make(int x) { return S(x); }
}
int main() {
 return copy_only::take(copy_only::make(7)) + movable::take(movable::make(8)) + destruction::take(destruction::make(9)) - 24;
}
