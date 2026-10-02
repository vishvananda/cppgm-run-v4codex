// PA34 reducer from PostTokenCursor::next and encoding_type.
// C++11 [stmt.switch]/2 permits enumeration conditions; [conv.prom] promotes
// unscoped enumerations only, regardless of their underlying integer width.
enum class byte_kind : unsigned char { zero, high = 255 };
enum class signed_kind : signed char { negative = -4, positive = 4 };
enum unscoped : unsigned char { small = 3 };
int pick(byte_kind value) {
    switch (value) { case byte_kind::zero: return 1; case byte_kind::high: return 2; }
    return 0;
}
int sign(signed_kind value) {
    switch (value) { case signed_kind::negative: return 3; default: return 4; }
}
int main() {
    if (pick(byte_kind::zero) != 1 || pick(byte_kind::high) != 2) return 1;
    if (sign(signed_kind::negative) != 3 || sign(signed_kind::positive) != 4) return 2;
    switch (small) { case 3: return 0; default: return 3; }
}
