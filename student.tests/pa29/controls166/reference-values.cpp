constexpr bool active(){return __builtin_is_constant_evaluated();}
enum Value { selected = 7, fallback = 9 };
template<int N> int from_argument(){const int& r = active() ? N : 0; const int& alias = r; return r+alias;}
int main(){
    const Value& e = active() ? selected : fallback;
    return from_argument<3>() == 6 && from_argument<5>() == 10 && e == selected ? 0 : 1;
}
