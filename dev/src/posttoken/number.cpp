// Floating extraction follows the CPPGM PA2 starter; see NOTICE.
#include "posttoken/number.h"
#include "support/extended_float.h"
#include <cstring>
#include <cstdlib>
#include <istream>
#include <limits>
#include <streambuf>

namespace cppgm {

std::uint32_t next_codepoint(TextView text, std::size_t& position)
{
    unsigned char c = text.data[position++];
    if (c < 128) return c;
    unsigned count = c < 0xe0 ? 1 : c < 0xf0 ? 2 : 3;
    std::uint32_t value = c & (count == 1 ? 31 : count == 2 ? 15 : 7);
    while (count--) value = (value << 6) | (text.data[position++] & 63);
    return value;
}

bool valid_ud_suffix(TextView text)
{
    // Non-underscore suffixes are reserved and invalid as PA2 expressions.
    if (!text.size || text.data[0] != '_') return false;
    std::size_t p = 1;
    while (p < text.size)
        if (!identifier_continue(next_codepoint(text, p))) return false;
    return true;
}

// The starter's stream extraction, using a borrowed buffer instead of an
// istringstream's owning copy. Only invoked after full grammar validation.
class NumberBuffer : public std::streambuf {
public:
    explicit NumberBuffer(TextView text) {
        char* begin = const_cast<char*>(text.data);
        setg(begin, begin, begin + text.size);
    }
};

template<class T> static T decode_floating(TextView text)
{
    NumberBuffer buffer(text);
    std::istream in(&buffer);
    T value = 0;
    in >> value;
    return value;
}

static float PA2Decode_float(TextView s) { return decode_floating<float>(s); }
static double PA2Decode_double(TextView s) { return decode_floating<double>(s); }
static long double PA2Decode_long_double(TextView s) { return decode_floating<long double>(s); }

static void floating_value(PostToken& token, TextView prefix, TextView suffix, bool hexadecimal = false)
{
    token.kind = PostTokenKind::literal;
    std::string hex;
    if (hexadecimal) hex.assign(prefix.data,prefix.size);
    if (suffix.equals("q") || suffix.equals("Q") || suffix.equals("f128") || suffix.equals("F128")) {
        token.type = FT_FLOAT128; auto value = parse_extended_float(prefix);
        std::memcpy(token.scalar.data(),&value,16);
    } else if (suffix.equals("f16") || suffix.equals("F16")) {
        token.type = FT_FLOAT16; auto value = half_bits(parse_extended_float(prefix,11));
        std::memcpy(token.scalar.data(),&value,2);
    } else if (suffix.equals("f") || suffix.equals("F")) {
        token.type = FT_FLOAT;
        float value = hexadecimal ? std::strtof(hex.c_str(),nullptr) : PA2Decode_float(prefix);
        std::memcpy(token.scalar.data(), &value, 4);
    } else if (suffix.equals("l") || suffix.equals("L")) {
        token.type = FT_LONG_DOUBLE;
        long double value = hexadecimal ? std::strtold(hex.c_str(),nullptr) : PA2Decode_long_double(prefix);
        // x86-64's 80-bit value has six ABI padding bytes, canonicalized to 0.
        static_assert(sizeof(value) == 16, "PA2 requires the x86-64 host ABI");
        std::memcpy(token.scalar.data(), &value, 10);
    } else if (!suffix.size) {
        token.type = FT_DOUBLE;
        double value = hexadecimal ? std::strtod(hex.c_str(),nullptr) : PA2Decode_double(prefix);
        std::memcpy(token.scalar.data(), &value, 8);
    } else token.kind = PostTokenKind::invalid;
}

static bool integer_suffix(TextView s, bool& uns, unsigned& rank)
{
    std::size_t p = 0;
    uns = false; rank = 0;
    if (p < s.size && (s.data[p] == 'u' || s.data[p] == 'U')) { uns = true; ++p; }
    if (p < s.size && (s.data[p] == 'l' || s.data[p] == 'L')) {
        char c = s.data[p++]; rank = 1;
        if (p < s.size && s.data[p] == c) { ++p; rank = 2; }
    }
    if (!uns && p < s.size && (s.data[p] == 'u' || s.data[p] == 'U')) { uns = true; ++p; }
    return p == s.size;
}

static void integer_value(PostToken& token, TextView digits, TextView suffix, unsigned base)
{
    bool uns;
    unsigned rank;
    if (!integer_suffix(suffix, uns, rank)) return;
    std::uint64_t value = 0;
    for (std::size_t p = 0; p < digits.size; ++p) {
        unsigned digit = hex_value(digits.data[p]);
        if (value > (std::numeric_limits<std::uint64_t>::max() - digit) / base) return;
        value = value * base + digit;
    }
    static const EFundamentalType signed_types[] = { FT_INT, FT_LONG_INT, FT_LONG_LONG_INT };
    static const EFundamentalType unsigned_types[] = { FT_UNSIGNED_INT, FT_UNSIGNED_LONG_INT, FT_UNSIGNED_LONG_LONG_INT };
    for (; rank < 3; ++rank) {
        unsigned bits = rank == 0 ? 32 : 64;
        std::uint64_t signed_max = (std::uint64_t(1) << (bits - 1)) - 1;
        if (!uns && value <= signed_max) token.type = signed_types[rank];
        else if ((uns || base != 10) && (bits == 64 || value <= 0xffffffffu))
            token.type = unsigned_types[rank];
        else continue;
        token.kind = PostTokenKind::literal;
        for (unsigned i = 0; i < bits / 8; ++i)
            token.scalar[i] = static_cast<char>(value >> (8 * i));
        return;
    }
}

void decode_number(PostToken& token, IdentifierTable& identifiers, NumberDomain domain, bool hosted)
{
    TextView s = token.source.spelling;
    std::size_t p = 0, digit_begin = 0;
    unsigned base = 10;
    bool floating = false, hexadecimal = false;
    if (s.size >= 2 && s.data[0] == '0' && (s.data[1] == 'x' || s.data[1] == 'X')) {
        base = 16; p = digit_begin = 2;
        while (p < s.size && hex_value(s.data[p]) >= 0) ++p;
        bool digits = p != digit_begin;
        if (hosted && p < s.size && s.data[p] == '.') {
            floating = true; ++p; auto start = p;
            while (p < s.size && hex_value(s.data[p]) >= 0) ++p;
            digits |= p != start;
        }
        if (!digits) return;
        if (hosted && p < s.size && (s.data[p] == 'p' || s.data[p] == 'P')) {
            floating = hexadecimal = true; ++p;
            if (p < s.size && (s.data[p] == '+' || s.data[p] == '-')) ++p;
            auto start = p;
            while (p < s.size && decimal_digit(s.data[p])) ++p;
            if (p == start) return;
        } else if (floating) return;
    } else if (hosted && s.size >= 2 && s.data[0] == '0' && (s.data[1] == 'b' || s.data[1] == 'B')) {
        base = 2; p = digit_begin = 2;
        while (p < s.size && (s.data[p] == '0' || s.data[p] == '1')) ++p;
        if (p == digit_begin) return;
    } else {
        while (p < s.size && decimal_digit(s.data[p])) ++p;
        bool digits = p != 0;
        if (p < s.size && s.data[p] == '.') {
            floating = true; ++p;
            std::size_t start = p;
            while (p < s.size && decimal_digit(s.data[p])) ++p;
            if (!digits && p == start) return;
        }
        if (p < s.size && (s.data[p] == 'e' || s.data[p] == 'E')) {
            floating = true; ++p;
            if (p < s.size && (s.data[p] == '+' || s.data[p] == '-')) ++p;
            std::size_t start = p;
            while (p < s.size && decimal_digit(s.data[p])) ++p;
            if (p == start) return;
        }
        if (!floating && s.data[0] == '0') {
            base = 8;
            for (std::size_t i = 0; i < p; ++i) if (s.data[i] > '7') return;
        }
    }
    TextView suffix(s.data + p, s.size - p);
    if (!hosted && (suffix.equals("q") || suffix.equals("Q") || suffix.equals("f16") || suffix.equals("F16") || suffix.equals("f128") || suffix.equals("F128"))) return;
    EFundamentalType extended_type=FT_VOID;
    if (hosted && floating) {
        if (suffix.equals("f16") || suffix.equals("F16")) extended_type=FT_FLOAT16;
        if (suffix.equals("f32") || suffix.equals("F32")) extended_type=FT_FLOAT32;
        if (suffix.equals("f64") || suffix.equals("F64")) extended_type=FT_FLOAT64;
        if (suffix.equals("f32x") || suffix.equals("F32x")) extended_type=FT_FLOAT32X;
        if (suffix.equals("f64x") || suffix.equals("F64x")) extended_type=FT_FLOAT64X;
        if (suffix.equals("f128") || suffix.equals("F128")) extended_type=FT_STDFLOAT128;

        if (suffix.equals("F64x") || suffix.equals("f64x") || suffix.equals("w") || suffix.equals("W")) suffix = TextView("L",1);
        else if (suffix.equals("F32") || suffix.equals("f32")) suffix = TextView("F",1);
        else if (suffix.equals("F64") || suffix.equals("f64") || suffix.equals("F32x") || suffix.equals("f32x")) suffix = TextView();
    }
    if (domain == NumberDomain::integral) {
        // All floating and user-defined numbers are invalid in a controlling
        // expression, even in unselected arms. No float extraction or suffix
        // interning is needed to prove rejection. PA1 already lexed the token.
        if (!floating)
            integer_value(token, TextView(s.data + digit_begin, p - digit_begin), suffix, base);
        return;
    }
    token.literal = floating ? LiteralKind::floating : LiteralKind::integer;
    token.prefix = TextView(s.data, p);
    if (valid_ud_suffix(suffix)) {
        token.suffix = identifiers.intern(suffix);
        // Retain both forms once: literal templates need the source characters,
        // while cooked operators consume the phase-7 numeric value. Overflow
        // only rejects a later cooked call; raw/template operators can use it.
        if (floating) floating_value(token,token.prefix,TextView("L",1),hexadecimal);
        else {
            integer_value(token,TextView(s.data+digit_begin,p-digit_begin),TextView(),base);
            if (token.kind != PostTokenKind::literal)
                integer_value(token,TextView(s.data+digit_begin,p-digit_begin),TextView("ULL",3),base);
        }
        token.cooked_valid = token.kind == PostTokenKind::literal;
        token.kind = PostTokenKind::user_literal;
    } else if (floating) floating_value(token, token.prefix, suffix, hexadecimal);
    else integer_value(token, TextView(s.data + digit_begin, p - digit_begin), suffix, base);
    if (token.kind==PostTokenKind::literal && extended_type!=FT_VOID) token.type=extended_type;
}

}
